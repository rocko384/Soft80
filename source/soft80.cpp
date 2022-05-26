#include "soft80.h"

Soft80::Soft80() {
	execution_thread = std::thread(&Soft80::executor, this);
}

Soft80::~Soft80() {
	execution_thread.join();
}

bool Soft80::read_busack() {
	return busack;
}

bool Soft80::read_halt() {
	return halt;
}

bool Soft80::read_iorq() {
	return iorq;
}

bool Soft80::read_m1() {
	return m1;
}

bool Soft80::read_mreq() {
	return mreq;
}

bool Soft80::read_rd() {
	return rd;
}

bool Soft80::read_wr() {
	return wr;
}

bool Soft80::read_rfsh() {
	return rfsh;
}

void Soft80::set_busreq_source(bool& b) {
	busreq_source = &b;
}

void Soft80::set_int_source(bool& b) {
	int_source = &b;
}

void Soft80::set_nmi_source(bool& b) {
	nmi_source = &b;
}

void Soft80::set_reset_source(bool& b) {
	reset_source = &b;
}

void Soft80::set_wait_source(bool& b) {
	wait_source = &b;
}

void Soft80::cycle_clock() {
	should_cycle = true;

	total_t_cycles++;
	current_t_cycles++;
}

void Soft80::kill() {
	should_executor_exit = true;
}

void Soft80::executor() {
	while (!should_executor_exit) {

		if (!halt) {
			fetch_opcode();
		}
		else {
			wait_next_clock();
			wait_next_clock();
			wait_next_clock();
			wait_next_clock();
			current_instruction = Instruction{};
		}

		if (current_instruction) {
			execute_instruction();
		}
	}
}

void Soft80::wait_next_clock() {
	while (!should_cycle) {
		if (should_executor_exit) {
			exit(0);
		}

		std::this_thread::yield();
	}

	should_cycle = false;
}

void Soft80::fetch_opcode() {
	wait_next_clock();

	iorq = false;
	m1 = true;
	mreq = true;
	rd = true;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = false;
	m1 = true;
	mreq = true;
	rd = true;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = false;
	m1 = false;
	mreq = true;
	rd = false;
	wr = false;
	rfsh = true;

	wait_next_clock();

	iorq = false;
	m1 = false;
	mreq = true;
	rd = false;
	wr = false;
	rfsh = true;

	uint8_t read_byte = memory.read(registers.PC);

	current_instruction = decoder.decode(read_byte);

	registers.PC++;
}

uint8_t Soft80::read_memory(uint16_t address) {
	wait_next_clock();

	address_bus = address;

	iorq = false;
	m1 = false;
	mreq = true;
	rd = true;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = false;
	m1 = false;
	mreq = true;
	rd = true;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = false;
	m1 = false;
	mreq = false;
	rd = true;
	wr = false;
	rfsh = false;

	return memory.read(address);
}

void Soft80::write_memory(uint16_t address, uint8_t value) {
	wait_next_clock();

	address_bus = address;
	data_bus = value;

	iorq = false;
	m1 = false;
	mreq = true;
	rd = false;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = false;
	m1 = false;
	mreq = true;
	rd = false;
	wr = true;
	rfsh = false;

	wait_next_clock();

	iorq = false;
	m1 = false;
	mreq = false;
	rd = false;
	wr = true;
	rfsh = false;

	memory.write(address, value);
}

uint8_t Soft80::read_io(uint8_t port_lo, uint8_t port_hi) {
	wait_next_clock();

	address_bus = (static_cast<uint16_t>(port_hi) << 8) | port_lo;

	iorq = false;
	m1 = false;
	mreq = false;
	rd = false;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = true;
	m1 = false;
	mreq = false;
	rd = true;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = true;
	m1 = false;
	mreq = false;
	rd = true;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = true;
	m1 = false;
	mreq = false;
	rd = true;
	wr = false;
	rfsh = false;

	return devices.read(port_lo, port_hi);
}

void Soft80::write_io(uint8_t port_lo, uint8_t port_hi, uint8_t value) {
	wait_next_clock();

	address_bus = (static_cast<uint16_t>(port_hi) << 8) | port_lo;
	data_bus = value;

	iorq = false;
	m1 = false;
	mreq = false;
	rd = false;
	wr = false;
	rfsh = false;

	wait_next_clock();

	iorq = true;
	m1 = false;
	mreq = false;
	rd = false;
	wr = true;
	rfsh = false;

	wait_next_clock();

	iorq = true;
	m1 = false;
	mreq = false;
	rd = false;
	wr = true;
	rfsh = false;

	wait_next_clock();

	iorq = true;
	m1 = false;
	mreq = false;
	rd = false;
	wr = true;
	rfsh = false;

	devices.write(port_lo, port_hi, value);
}

void Soft80::bus_acknowledge() {
	wait_next_clock();

	address_bus = 0;
	data_bus = 0;

	busack = true;

	while (*busreq_source) {
		wait_next_clock();
	}

	busack = false;
}

void Soft80::int_acknowledge() {

}

void Soft80::nmi_acknowledge() {

}

void Soft80::execute_instruction() {

	uint32_t operand1{ 0 };
	uint32_t operand2{ 0 };
	uint32_t result{ 0 };

	uint16_t dest_value{ 0 };
	uint16_t source_value{ 0 };

	if (current_instruction->dest == RegisterFile::Names::Immediate) {
		dest_value = current_instruction->imm;
	}
	else {
		auto reg_val = registers.get_value(current_instruction->dest);

		if (reg_val) {
			dest_value = reg_val.value();
		}
	}

	if (current_instruction->source == RegisterFile::Names::Immediate) {
		source_value = current_instruction->imm;
	}
	else {
		auto reg_val = registers.get_value(current_instruction->source);

		if (reg_val) {
			source_value = reg_val.value();
		}
	}

	if (current_instruction->addr_dest) {
		operand1 = read_memory(dest_value + current_instruction->displacement);
	}
	else {
		operand1 = dest_value;
	}

	if (current_instruction->addr_source) {
		operand2 = read_memory(source_value + current_instruction->displacement);
	}
	else {
		operand2 = source_value;
	}

	Flags flags = parse_flags(registers.main.F);

	FlagNames out_flags = FlagNames::None;

	bool is_io = false;

	switch (current_instruction->name) {

	case Instruction::Names::ADC:
		result = operand1 + operand2;

		if (flags.carry) {
			result += 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::ADD:
		result = operand1 + operand2;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::AND:
		result = operand1 & operand2;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarrySet
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::CarryReset;

		break;

	case Instruction::Names::BIT:
	{
		uint32_t bit = 1 << operand1;

		bool is_set = bit & operand2;

		out_flags =
			FlagNames::F5
			| FlagNames::HalfCarrySet
			| FlagNames::F3
			| FlagNames::SubtractReset;

		if (is_set) {
			out_flags |= FlagNames::ZeroReset;
			out_flags |= FlagNames::OverflowReset;

			if (operand1 == 7) {
				out_flags |= FlagNames::SignSet;
			}
		}
		else {
			out_flags |= FlagNames::ZeroSet;
			out_flags |= FlagNames::OverflowSet;
		}

		break;
	}

	case Instruction::Names::CALL:
	{
		bool take = true;

		switch (current_instruction->condition) {
		case Instruction::Conditions::NZ:
			take = !flags.zero;
			break;
		case Instruction::Conditions::Z:
			take = flags.zero;
			break;
		case Instruction::Conditions::NC:
			take = !flags.carry;
			break;
		case Instruction::Conditions::C:
			take = flags.carry;
			break;
		case Instruction::Conditions::PO:
			take = !flags.parity;
			break;
		case Instruction::Conditions::PE:
			take = flags.parity;
			break;
		case Instruction::Conditions::P:
			take = !flags.sign;
			break;
		case Instruction::Conditions::M:
			take = flags.sign;
			break;
		}

		if (take) {
			uint8_t PC_high = (registers.PC & 0xFF00) >> 8;
			uint8_t PC_low = registers.PC & 0x00FF;

			registers.SP--;
			write_memory(registers.SP, PC_high);
			registers.SP--;
			write_memory(registers.SP, PC_low);

			registers.PC = dest_value;
		}

		break;
	}

	case Instruction::Names::CCF:

		out_flags = FlagNames::SubtractReset;

		if (flags.carry) {
			out_flags |= FlagNames::CarryReset;
			out_flags |= FlagNames::HalfCarrySet;
		}
		else {
			out_flags |= FlagNames::CarrySet;
			out_flags |= FlagNames::HalfCarryReset;
		}

		break;

	case Instruction::Names::CP:
		result = operand2;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractSet
			| FlagNames::Carry;

		break;

	case Instruction::Names::CPD:
	{
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;
		uint8_t A = registers.main.A;

		uint8_t val = read_memory(HL);

		uint8_t res = A - val;

		registers.main.HL = HL - 1;
		registers.main.BC = BC - 1;

		bool is_neg = res & 0x80;
		bool halfc = (A & 0x10) && (val & 0x10);

		out_flags =
			FlagNames::SubtractSet;

		if (is_neg) {
			out_flags |= FlagNames::SignSet;
		}
		else {
			out_flags |= FlagNames::SignReset;
		}

		if (res == 0) {
			out_flags |= FlagNames::ZeroSet;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		if (halfc) {
			out_flags |= FlagNames::HalfCarrySet;
		}
		else {
			out_flags |= FlagNames::HalfCarryReset;
		}

		if (registers.main.BC == 0) {
			out_flags |= FlagNames::OverflowReset;
		}
		else {
			out_flags |= FlagNames::OverflowSet;
		}

		break;
	}

	case Instruction::Names::CPDR:
	{
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;
		uint8_t A = registers.main.A;

		uint8_t val = read_memory(HL);

		uint8_t res = A - val;

		registers.main.HL = HL - 1;
		registers.main.BC = BC - 1;

		bool is_neg = res & 0x80;
		bool halfc = (A & 0x10) && (val & 0x10);

		out_flags =
			FlagNames::SubtractSet;

		bool should_repeat = true;

		if (is_neg) {
			out_flags |= FlagNames::SignSet;
		}
		else {
			out_flags |= FlagNames::SignReset;
		}

		if (res == 0) {
			out_flags |= FlagNames::ZeroSet;

			should_repeat = false;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		if (halfc) {
			out_flags |= FlagNames::HalfCarrySet;
		}
		else {
			out_flags |= FlagNames::HalfCarryReset;
		}

		if (registers.main.BC == 0) {
			out_flags |= FlagNames::OverflowReset;

			should_repeat = false;
		}
		else {
			out_flags |= FlagNames::OverflowSet;
		}

		if (should_repeat) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::CPI:
	{
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;
		uint8_t A = registers.main.A;

		uint8_t val = read_memory(HL);

		uint8_t res = A - val;

		registers.main.HL = HL + 1;
		registers.main.BC = BC - 1;

		bool is_neg = res & 0x80;
		bool halfc = (A & 0x10) && (val & 0x10);

		out_flags =
			FlagNames::SubtractSet;

		if (is_neg) {
			out_flags |= FlagNames::SignSet;
		}
		else {
			out_flags |= FlagNames::SignReset;
		}

		if (res == 0) {
			out_flags |= FlagNames::ZeroSet;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		if (halfc) {
			out_flags |= FlagNames::HalfCarrySet;
		}
		else {
			out_flags |= FlagNames::HalfCarryReset;
		}

		if (registers.main.BC == 0) {
			out_flags |= FlagNames::OverflowReset;
		}
		else {
			out_flags |= FlagNames::OverflowSet;
		}

		break;
	}

	case Instruction::Names::CPIR:
	{
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;
		uint8_t A = registers.main.A;

		uint8_t val = read_memory(HL);

		uint8_t res = A - val;

		registers.main.HL = HL + 1;
		registers.main.BC = BC - 1;

		bool is_neg = res & 0x80;
		bool halfc = (A & 0x10) && (val & 0x10);

		out_flags =
			FlagNames::SubtractSet;

		bool should_repeat = true;

		if (is_neg) {
			out_flags |= FlagNames::SignSet;
		}
		else {
			out_flags |= FlagNames::SignReset;
		}

		if (res == 0) {
			out_flags |= FlagNames::ZeroSet;

			should_repeat = false;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		if (halfc) {
			out_flags |= FlagNames::HalfCarrySet;
		}
		else {
			out_flags |= FlagNames::HalfCarryReset;
		}

		if (registers.main.BC == 0) {
			out_flags |= FlagNames::OverflowReset;

			should_repeat = false;
		}
		else {
			out_flags |= FlagNames::OverflowSet;
		}

		if (should_repeat) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::CPL:

		result = ~operand1;

		out_flags =
			FlagNames::HalfCarrySet
			| FlagNames::SubtractSet;

		break;

	case Instruction::Names::DAA:
	{
		uint8_t upper = static_cast<uint8_t>(operand1 & 0x000000F0) >> 4;
		uint8_t lower = static_cast<uint8_t>(operand1 & 0x0000000F);

		result = operand1;

		if (lower > 0x09 || flags.half_carry) {
			result += 0x06;
		}

		if (upper > 0x09 || flags.carry) {
			result += 0x60;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::Carry;

		break;
	}

	case Instruction::Names::DEC:

		result = operand1 - 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractSet;

		break;

	case Instruction::Names::DI:
		iff1 = false;
		iff2 = false;
		break;

	case Instruction::Names::DJNZ:

		registers.main.B = registers.main.B - 1;

		if (registers.main.B != 0) {
			registers.PC = registers.PC + current_instruction->displacement;
		}

		break;

	case Instruction::Names::EI:
		iff1 = true;
		iff2 = true;
		break;

	case Instruction::Names::EX:

		registers.set_value(current_instruction->source, operand1);
		result = operand2;

		break;

	case Instruction::Names::EXX:

		std::swap(registers.main.BC, registers.alt.BC);
		std::swap(registers.main.DE, registers.alt.DE);
		std::swap(registers.main.HL, registers.alt.HL);

		break;

	case Instruction::Names::HALT:

		halt = true;

		break;

	case Instruction::Names::IM:

		if (current_instruction->imm == 0) {
			interrupt_mode = 0;
		}
		else if (current_instruction->imm == 2) {
			interrupt_mode = 1;
		}
		else if (current_instruction->imm == 3) {
			interrupt_mode = 2;
		}

		break;

	case Instruction::Names::IN:
	{
		is_io = true;

		uint8_t low = registers.main.C;
		uint8_t high = registers.main.B;

		if (current_instruction->source == RegisterFile::Names::Immediate) {
			low = current_instruction->imm_low;
			high = registers.main.A;
		}

		result = read_io(low, high);

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		break;
	}

	case Instruction::Names::INC:
		result = operand1 + 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractReset;

		break;

	case Instruction::Names::IND:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_io(C, B);

		write_memory(HL, val);

		registers.main.B = B - 1;
		registers.main.HL = HL - 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B == 0) {
			out_flags |= FlagNames::ZeroSet;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		break;
	}

	case Instruction::Names::INDR:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_io(C, B);

		write_memory(HL, val);

		registers.main.B = B - 1;
		registers.main.HL = HL - 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::ZeroSet
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B != 0) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::INI:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_io(C, B);

		write_memory(HL, val);

		registers.main.B = B - 1;
		registers.main.HL = HL + 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B == 0) {
			out_flags |= FlagNames::ZeroSet;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		break;
	}

	case Instruction::Names::INIR:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_io(C, B);

		write_memory(HL, val);

		registers.main.B = B - 1;
		registers.main.HL = HL + 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::ZeroSet
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B != 0) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::JP:
	{
		bool take = true;

		switch (current_instruction->condition) {
		case Instruction::Conditions::NZ:
			take = !flags.zero;
			break;
		case Instruction::Conditions::Z:
			take = flags.zero;
			break;
		case Instruction::Conditions::NC:
			take = !flags.carry;
			break;
		case Instruction::Conditions::C:
			take = flags.carry;
			break;
		case Instruction::Conditions::PO:
			take = !flags.parity;
			break;
		case Instruction::Conditions::PE:
			take = flags.parity;
			break;
		case Instruction::Conditions::P:
			take = !flags.sign;
			break;
		case Instruction::Conditions::M:
			take = flags.sign;
			break;
		}

		if (take) {
			registers.PC = dest_value;
		}

		break;
	}

	case Instruction::Names::JR:

		registers.PC = registers.PC + current_instruction->displacement;

		break;

	case Instruction::Names::LD:

		if (current_instruction->dest == RegisterFile::Names::A
			&& (current_instruction->source == RegisterFile::Names::I
				|| current_instruction->source == RegisterFile::Names::R)) {

			out_flags =
				FlagNames::Sign
				| FlagNames::Zero
				| FlagNames::F5
				| FlagNames::HalfCarryReset
				| FlagNames::F3
				| FlagNames::Parity
				| FlagNames::SubtractReset;

			if (iff2) {
				out_flags |= FlagNames::OverflowSet;
			}
			else {
				out_flags |= FlagNames::OverflowReset;
			}
		}

		result = operand2;

		break;

	case Instruction::Names::LDD:
	{
		uint16_t DE = registers.main.DE;
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;

		uint8_t val = read_memory(HL);
		write_memory(DE, val);

		registers.main.DE = DE - 1;
		registers.main.HL = HL - 1;
		registers.main.BC = BC - 1;

		out_flags =
			FlagNames::HalfCarryReset
			| FlagNames::SubtractReset;

		if (registers.main.BC == 0) {
			out_flags |= FlagNames::OverflowReset;
		}
		else {
			out_flags |= FlagNames::OverflowSet;
		}

		break;
	}
		
	case Instruction::Names::LDDR:
	{
		uint16_t DE = registers.main.DE;
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;

		uint8_t val = read_memory(HL);
		write_memory(DE, val);

		registers.main.DE = DE - 1;
		registers.main.HL = HL - 1;
		registers.main.BC = BC - 1;

		out_flags =
			FlagNames::HalfCarryReset
			| FlagNames::OverflowReset
			| FlagNames::SubtractReset;

		if (registers.main.BC != 0) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::LDI:
	{
		uint16_t DE = registers.main.DE;
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;

		uint8_t val = read_memory(HL);
		write_memory(DE, val);

		registers.main.DE = DE + 1;
		registers.main.HL = HL + 1;
		registers.main.BC = BC - 1;

		out_flags =
			FlagNames::HalfCarryReset
			| FlagNames::SubtractReset;

		if (registers.main.BC == 0) {
			out_flags |= FlagNames::OverflowReset;
		}
		else {
			out_flags |= FlagNames::OverflowSet;
		}

		break;
	}

	case Instruction::Names::LDIR:
	{
		uint16_t DE = registers.main.DE;
		uint16_t HL = registers.main.HL;
		uint16_t BC = registers.main.BC;

		uint8_t val = read_memory(HL);
		write_memory(DE, val);

		registers.main.DE = DE + 1;
		registers.main.HL = HL + 1;
		registers.main.BC = BC - 1;

		out_flags =
			FlagNames::HalfCarryReset
			| FlagNames::OverflowReset
			| FlagNames::SubtractReset;

		if (registers.main.BC != 0) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::NEG:

		result = -operand1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractSet
			| FlagNames::Carry;

		break;

	case Instruction::Names::NOP:
	
		break;

	case Instruction::Names::OR:
		result = operand1 | operand2;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::CarryReset;

		break;

	case Instruction::Names::OTDR:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_memory(HL);

		write_io(C, B, val);

		registers.main.B = B - 1;
		registers.main.HL = HL - 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::ZeroSet
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B != 0) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::OTIR:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_memory(HL);

		write_io(C, B, val);

		registers.main.B = B - 1;
		registers.main.HL = HL + 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::ZeroSet
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B != 0) {
			registers.PC = registers.PC - 2;
		}

		break;
	}

	case Instruction::Names::OUT:
	{
		is_io = true;

		uint8_t low = registers.main.C;
		uint8_t high = registers.main.B;

		if (current_instruction->dest == RegisterFile::Names::Immediate) {
			low = current_instruction->imm_low;
			high = registers.main.A;
		}

		write_io(low, high, registers.main.A);

		break;
	}

	case Instruction::Names::OUTD:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_memory(HL);

		write_io(C, B, val);

		registers.main.B = B - 1;
		registers.main.HL = HL - 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B == 0) {
			out_flags |= FlagNames::ZeroSet;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		break;
	}

	case Instruction::Names::OUTI:
	{
		is_io = true;

		uint8_t C = registers.main.C;
		uint8_t B = registers.main.B;
		uint16_t HL = registers.main.HL;

		uint8_t val = read_memory(HL);

		write_io(C, B, val);

		registers.main.B = B - 1;
		registers.main.HL = HL + 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::F5
			| FlagNames::F3
			| FlagNames::SubtractSet;

		if (registers.main.B == 0) {
			out_flags |= FlagNames::ZeroSet;
		}
		else {
			out_flags |= FlagNames::ZeroReset;
		}

		break;
	}

	case Instruction::Names::POP:
	{
		uint16_t low = read_memory(registers.SP);
		registers.SP++;
		uint16_t high = read_memory(registers.SP);
		registers.SP++;

		result = low | (high << 8);

		break;
	}

	case Instruction::Names::PUSH:
	{
		uint8_t low = operand2 & 0x000000FF;
		uint8_t high = (operand2 & 0x0000FF00) >> 8;

		registers.SP--;
		write_memory(registers.SP, high);
		registers.SP--;
		write_memory(registers.SP, low);

		break;
	}

	case Instruction::Names::RES:
	{
		uint8_t bit = 1 << operand1;
		uint8_t value = (operand2 | bit) ^ bit;

		if (current_instruction->addr_source) {
			write_memory(source_value, value);
		}
		else {
			registers.set_value(current_instruction->source, value);
		}

		break;
	}

	case Instruction::Names::RET:
	{
		bool take = true;

		switch (current_instruction->condition) {
		case Instruction::Conditions::NZ:
			take = !flags.zero;
			break;
		case Instruction::Conditions::Z:
			take = flags.zero;
			break;
		case Instruction::Conditions::NC:
			take = !flags.carry;
			break;
		case Instruction::Conditions::C:
			take = flags.carry;
			break;
		case Instruction::Conditions::PO:
			take = !flags.parity;
			break;
		case Instruction::Conditions::PE:
			take = flags.parity;
			break;
		case Instruction::Conditions::P:
			take = !flags.sign;
			break;
		case Instruction::Conditions::M:
			take = flags.sign;
			break;
		}

		if (take) {
			uint16_t low = read_memory(registers.SP);
			registers.SP++;
			uint16_t high = read_memory(registers.SP);
			registers.SP++;

			uint16_t addr = low | (high << 8);

			registers.PC = addr;
		}

		break;
	}

	case Instruction::Names::RETI:
	{
		uint16_t low = read_memory(registers.SP);
		registers.SP++;
		uint16_t high = read_memory(registers.SP);
		registers.SP++;

		uint16_t addr = low | (high << 8);

		registers.PC = addr;

		break;
	}

	case Instruction::Names::RETN:
	{
		uint16_t low = read_memory(registers.SP);
		registers.SP++;
		uint16_t high = read_memory(registers.SP);
		registers.SP++;

		uint16_t addr = low | (high << 8);

		registers.PC = addr;

		iff1 = iff2;

		break;
	}

	case Instruction::Names::RL:

		result = operand1 << 1;

		if (flags.carry) {
			result |= 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::RLA:

		result = operand1 << 1;

		out_flags =
			FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::RLC:
		
		result = operand1 << 1;

		if (operand1 & 0xFFFFFF00) {
			result |= 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::RLCA:

		result = operand1 << 1;

		if (operand1 & 0xFFFFFF00) {
			result |= 1;
		}

		out_flags =
			FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::RLD:
	{
		uint16_t A = 0x000F & registers.main.A;
		uint16_t HL = read_memory(registers.main.HL);

		operand1 = (A << 8) | HL;

		result = operand1 << 4;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		break;
	}

	case Instruction::Names::RR:

		result = operand1 >> 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand1 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;

	case Instruction::Names::RRA:

		result = operand1 >> 1;

		out_flags =
			FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::SubtractReset;

		if (operand1 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;

	case Instruction::Names::RRC:

		result = operand1 >> 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand1 & 1) {
			result |= 0x80;

			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;

	case Instruction::Names::RRCA:

		result = operand1 >> 1;

		out_flags =
			FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::SubtractReset;

		if (operand1 & 1) {
			result |= 0x80;

			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;

	case Instruction::Names::RRD:
	{
		uint16_t A = 0x000F & registers.main.A;
		uint16_t HL = read_memory(registers.main.HL);

		operand1 = (A << 8) | HL;

		result = operand1 >> 4;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		break;
	}

	case Instruction::Names::RST:
	{
		uint8_t low = registers.PC & 0x00FF;
		uint8_t high = (registers.PC & 0xFF00) >> 8;

		registers.SP--;
		write_memory(registers.SP, high);
		registers.SP--;
		write_memory(registers.SP, low);

		registers.SP = current_instruction->imm * 8;

		break;
	}

	case Instruction::Names::SBC:

		result = operand1 - operand2;

		if (flags.carry) {
			result -= 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractSet
			| FlagNames::Carry;

		break;

	case Instruction::Names::SCF:

		out_flags =
			FlagNames::SubtractReset
			| FlagNames::CarrySet
			| FlagNames::HalfCarryReset;

		break;

	case Instruction::Names::SET:
	{
		uint8_t bit = 1 << operand1;
		uint8_t value = operand2 | bit;

		if (current_instruction->addr_source) {
			write_memory(source_value, value);
		}
		else {
			registers.set_value(current_instruction->source, value);
		}

		break;
	}

	case Instruction::Names::SLA:

		result = operand1 << 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::SRA:

		if (operand1 & 0xFFFFFF80) {
			result = (operand1 | 0xFFFFFF00) >> 1;
		}
		else {
			result = operand1 >> 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand1 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;

	case Instruction::Names::SLL:

		result = operand1 << 1;

		result |= 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;

	case Instruction::Names::SRL:
	
		result = operand1 >> 1;
	
		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand1 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;

	case Instruction::Names::SUB:

		result = operand1 - operand2;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarry
			| FlagNames::F3
			| FlagNames::Overflow
			| FlagNames::SubtractSet
			| FlagNames::Carry;

		break;

	case Instruction::Names::XOR:

		result = operand1 ^ operand2;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::CarryReset;

		break;

	case Instruction::Names::NONI:

		break;

	case Instruction::Names::RLCalt:
	{
		result = operand2 << 1;

		if (operand2 & 0xFFFFFF00) {
			result |= 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;
	}

	case Instruction::Names::RRCalt:
	{
		result = operand2 >> 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand2 & 1) {
			result |= 0x80;

			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;
	}

	case Instruction::Names::RLalt:
	{
		result = operand2 << 1;

		if (flags.carry) {
			result |= 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;
	}

	case Instruction::Names::RRalt:
	{
		result = operand2 >> 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand2 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;
	}

	case Instruction::Names::SLAalt:
	{
		result = operand2 << 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;
	}

	case Instruction::Names::SRAalt:
	{
		if (operand2 & 0xFFFFFF80) {
			result = (operand2 | 0xFFFFFF00) >> 1;
		}
		else {
			result = operand2 >> 1;
		}

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand2 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;
	}

	case Instruction::Names::SLLalt:
	{
		result = operand2 << 1;

		result |= 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset
			| FlagNames::Carry;

		break;
	}

	case Instruction::Names::SRLalt:
	{
		result = operand2 >> 1;

		out_flags =
			FlagNames::Sign
			| FlagNames::Zero
			| FlagNames::F5
			| FlagNames::HalfCarryReset
			| FlagNames::F3
			| FlagNames::Parity
			| FlagNames::SubtractReset;

		if (operand2 & 1) {
			out_flags |= FlagNames::CarrySet;
		}
		else {
			out_flags |= FlagNames::CarryReset;
		}

		break;
	}

	case Instruction::Names::RESalt:
	{
		uint8_t bit = 1 << current_instruction->imm;
		uint8_t value = (operand2 | bit) ^ bit;

		if (current_instruction->addr_source) {
			write_memory(source_value, value);
		}
		else {
			registers.set_value(current_instruction->source, value);
		}

		break;
	}

	case Instruction::Names::SETalt:
	{
		uint8_t bit = 1 << current_instruction->imm;
		uint8_t value = operand2 | bit;

		if (current_instruction->addr_source) {
			write_memory(source_value, value);
		}
		else {
			registers.set_value(current_instruction->source, value);
		}

		break;
	}
	}

	uint8_t out_flags_value = 0;

	if (is_flag_set(out_flags, FlagNames::Sign)) {
		bool is_neg = result & 0x00000080;

		if (RegisterFile::is_16bit(current_instruction->dest) && RegisterFile::is_16bit(current_instruction->source)) {
			is_neg = result & 0x00008000;
		}

		if (is_neg) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::Sign);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Sign);
		}
	}

	if (is_flag_set(out_flags, FlagNames::Zero)) {
		if (result == 0) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::Zero);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Zero);
		}
	}

	if (is_flag_set(out_flags, FlagNames::F5)) {
		if (result & 0x00000020) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::F5);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::F5);
		}
	}

	if (is_flag_set(out_flags, FlagNames::HalfCarry)) {
		if ((operand1 & 0x00000008) && (operand2 & 0x00000008)) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::HalfCarry);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::HalfCarry);
		}
	}

	if (is_flag_set(out_flags, FlagNames::F3)) {
		if (result & 0x00000008) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::F3);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::F3);
		}
	}

	if (is_flag_set(out_flags, FlagNames::Parity)) {

		size_t bit_count = 0;

		for (size_t i = 0; i < 8; i++) {
			uint8_t bit = 1 << i;

			if (result & bit) {
				bit_count++;
			}
		}

		if (bit_count % 2 == 0) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::Overflow);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Overflow);
		}

	}

	if (is_flag_set(out_flags, FlagNames::Overflow)) {
		bool is_overflow = result & 0xFFFFFFFF00;

		if (RegisterFile::is_16bit(current_instruction->dest) && RegisterFile::is_16bit(current_instruction->source)) {
			is_overflow = result & 0xFFFF0000;
		}

		if (is_overflow) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::Overflow);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Overflow);
		}
	}

	if (is_flag_set(out_flags, FlagNames::Carry)) {
		bool is_carry = result & 0xFFFFFF00;

		if (RegisterFile::is_16bit(current_instruction->dest) && RegisterFile::is_16bit(current_instruction->source)) {
			is_carry = result & 0xFFFF0000;
		}

		if (is_carry) {
			out_flags_value = set_flag_bit(out_flags_value, FlagBits::Carry);
		}
		else {
			out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Carry);
		}
	}

	if (is_flag_set(out_flags, FlagNames::SignSet)) {
		out_flags_value = set_flag_bit(out_flags_value, FlagBits::Sign);
	}

	if (is_flag_set(out_flags, FlagNames::SignReset)) {
		out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Sign);
	}

	if (is_flag_set(out_flags, FlagNames::ZeroSet)) {
		out_flags_value = set_flag_bit(out_flags_value, FlagBits::Zero);
	}

	if (is_flag_set(out_flags, FlagNames::ZeroReset)) {
		out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Zero);
	}

	if (is_flag_set(out_flags, FlagNames::HalfCarrySet)) {
		out_flags_value = set_flag_bit(out_flags_value, FlagBits::HalfCarry);
	}

	if (is_flag_set(out_flags, FlagNames::HalfCarryReset)) {
		out_flags_value = reset_flag_bit(out_flags_value, FlagBits::HalfCarry);
	}

	if (is_flag_set(out_flags, FlagNames::OverflowSet)) {
		out_flags_value = set_flag_bit(out_flags_value, FlagBits::Overflow);
	}

	if (is_flag_set(out_flags, FlagNames::OverflowReset)) {
		out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Overflow);
	}

	if (is_flag_set(out_flags, FlagNames::SubtractSet)) {
		out_flags_value = set_flag_bit(out_flags_value, FlagBits::Subtract);
	}

	if (is_flag_set(out_flags, FlagNames::SubtractReset)) {
		out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Subtract);
	}

	if (is_flag_set(out_flags, FlagNames::CarrySet)) {
		out_flags_value = set_flag_bit(out_flags_value, FlagBits::Carry);
	}

	if (is_flag_set(out_flags, FlagNames::CarryReset)) {
		out_flags_value = reset_flag_bit(out_flags_value, FlagBits::Carry);
	}

	registers.main.F = out_flags_value;

	if (!is_io) {
		if (current_instruction->addr_dest) {
			write_memory(dest_value, result);
		}
		else {
			registers.set_value(current_instruction->dest, result);
		}
	}

}