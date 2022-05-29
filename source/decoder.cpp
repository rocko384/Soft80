#include "decoder.h"

#include <functional>

struct InstructionDetails {
	Instruction::Names name{ Instruction::Names::NOP };
	RegisterFile::Names dest{ RegisterFile::Names::None };
	bool alt_dest{ false };
	bool addr_dest{ false };
	RegisterFile::Names source{ RegisterFile::Names::None };
	bool alt_source{ false };
	bool addr_source{ false };
	Instruction::Conditions condition{ Instruction::Conditions::None };

	bool needs_displacement{ false };
	uint8_t immediate_bytes{ 0 };
	uint8_t immediate_value{ 0 };
};

const std::function<InstructionDetails(Opcode)> NoPreX0ZTable[]{
	// z = 0
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		switch (o.y) {
		case 0:
			ret.name = Instruction::Names::NOP;
			break;
		case 1:
			ret.name = Instruction::Names::EX;
			ret.dest = RegisterFile::Names::A;
			ret.source = RegisterFile::Names::A;
			ret.alt_source = true;
			break;
		case 2:
			ret.name = Instruction::Names::DJNZ;
			ret.needs_displacement = true;
			break;
		case 3:
			ret.name = Instruction::Names::JR;
			ret.needs_displacement = true;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			ret.name = Instruction::Names::JR;
			ret.needs_displacement = true;
			ret.condition = ccTable[o.y - 4];
		}

		return ret;
	},

	// z = 1
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		if (o.q == 0) {
			ret.name = Instruction::Names::LD;
			ret.dest = rpTable[o.p];
			ret.source = RegisterFile::Names::Immediate;
			ret.immediate_bytes = 2;
		}
		else if (o.q == 1) {
			ret.name = Instruction::Names::ADD;
			ret.dest = RegisterFile::Names::HL;
			ret.source = rpTable[o.p];
		}

		return ret;
	},

	// z = 2
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::LD;
		ret.immediate_bytes = 2;

		RegisterFile::Names r1;
		RegisterFile::Names r2;

		switch (o.p) {
		case 0:
			r1 = RegisterFile::Names::BC;
			r2 = RegisterFile::Names::A;
			break;
		case 1:
			r1 = RegisterFile::Names::DE;
			r2 = RegisterFile::Names::A;
			break;
		case 2:
			r1 = RegisterFile::Names::Immediate;
			r2 = RegisterFile::Names::HL;
			break;
		case 3:
			r1 = RegisterFile::Names::Immediate;
			r2 = RegisterFile::Names::A;
			break;
		}

		if (o.q == 0) {
			ret.dest = r1;
			ret.addr_dest = true;
			ret.source = r2;
		}
		else {
			ret.dest = r2;
			ret.source = r1;
			ret.addr_source = true;
		}

		return ret;
	},

	// z = 3
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.dest = rpTable[o.p];

		if (o.q == 0) {
			ret.name = Instruction::Names::INC;
		}
		else {
			ret.name = Instruction::Names::DEC;
		}

		return ret;
	},

	// z = 4
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::INC;
		ret.dest = rTable[o.y];

		return ret;
	},

	// z = 5
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::DEC;
		ret.dest = rTable[o.y];

		return ret;
	},

	// z = 6
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::LD;
		ret.immediate_bytes = 1;
		ret.dest = rTable[o.y];
		ret.source = RegisterFile::Names::Immediate;

		return ret;
	},

	// z = 7
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.dest = RegisterFile::Names::A;

		switch (o.y) {
		case 0:
			ret.name = Instruction::Names::RLCA;
			break;
		case 1:
			ret.name = Instruction::Names::RRCA;
			break;
		case 2:
			ret.name = Instruction::Names::RLA;
			break;
		case 3:
			ret.name = Instruction::Names::RRA;
			break;
		case 4:
			ret.name = Instruction::Names::DAA;
			break;
		case 5:
			ret.name = Instruction::Names::CPL;
			break;
		case 6:
			ret.name = Instruction::Names::SCF;
			break;
		case 7:
			ret.name = Instruction::Names::CCF;
			break;
		}

		return ret;
	}
};

const std::function<InstructionDetails(Opcode)> NoPreX3ZTable[]{
	// z = 0
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::RET;
		ret.condition = ccTable[o.y];

		return ret;
	},

	// z = 1
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		if (o.q == 0) {
			ret.name = Instruction::Names::POP;
			ret.dest = rp2Table[o.p];
		}
		else {
			switch (o.p) {
			case 0:
				ret.name = Instruction::Names::RET;
				break;
			case 1:
				ret.name = Instruction::Names::EXX;
				break;
			case 2:
				ret.name = Instruction::Names::JP;
				ret.dest = RegisterFile::Names::HL;
				ret.addr_dest = true;
				break;
			case 3:
				ret.name = Instruction::Names::LD;
				ret.dest = RegisterFile::Names::SP;
				ret.source = RegisterFile::Names::HL;
				break;
			}
		}

		return ret;
	},

	// z = 2
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::JP;
		ret.condition = ccTable[o.y];
		ret.dest = RegisterFile::Names::Immediate;
		ret.immediate_bytes = 2;
		ret.addr_dest = true;

		return ret;
	},

	// z = 3
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		switch (o.y) {
		case 0:
			ret.name = Instruction::Names::JP;
			ret.dest = RegisterFile::Names::Immediate;
			ret.immediate_bytes = 2;
			ret.addr_dest = true;
		case 1:
			break;
		case 2:
			ret.name = Instruction::Names::OUT;
			ret.dest = RegisterFile::Names::Immediate;
			ret.immediate_bytes = 1;
			ret.addr_dest = true;
			ret.source = RegisterFile::Names::A;
			break;
		case 3:
			ret.name = Instruction::Names::IN;
			ret.dest = RegisterFile::Names::A;
			ret.source = RegisterFile::Names::Immediate;
			ret.immediate_bytes = 1;
			ret.addr_source = true;
			break;
		case 4:
			ret.name = Instruction::Names::EX;
			ret.dest = RegisterFile::Names::SP;
			ret.addr_dest = true;
			ret.source = RegisterFile::Names::HL;
			break;
		case 5:
			ret.name = Instruction::Names::EX;
			ret.dest = RegisterFile::Names::DE;
			ret.source = RegisterFile::Names::HL;
			break;
		case 6:
			ret.name = Instruction::Names::DI;
			break;
		case 7:
			ret.name = Instruction::Names::EI;
			break;
		}

		return ret;
	},

	// z = 4
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::CALL;
		ret.condition = ccTable[o.y];
		ret.dest = RegisterFile::Names::Immediate;
		ret.immediate_bytes = 2;
		ret.addr_dest = true;

		return ret;
	},

	// z = 5
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		if (o.q == 0) {
			ret.name = Instruction::Names::PUSH;
			ret.source = rp2Table[o.p];
		}
		else {
			switch (o.p) {
			case 0:
				ret.name = Instruction::Names::CALL;
				ret.dest = RegisterFile::Names::Immediate;
				ret.immediate_bytes = 2;
				break;
			case 1:
			case 2:
			case 3:
				break;
			}
		}

		return ret;
	},

	// z = 6
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = aluTable[o.y];
		ret.condition = ccTable[o.y];
		ret.dest = RegisterFile::Names::A;
		ret.source = RegisterFile::Names::Immediate;
		ret.immediate_bytes = 1;

		return ret;
	},

	// z = 7
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::RST;
		ret.dest = RegisterFile::Names::Immediate;
		ret.immediate_value = o.y * 8;

		return ret;
	}

};

const std::function<InstructionDetails(Opcode)> EDX1ZTable[]{
	// z = 0
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::IN;
		ret.source = RegisterFile::Names::BC;
		ret.addr_source = true;

		if (o.y != 6) {
			ret.dest = rTable[o.y];
		}
		else {
			ret.dest = RegisterFile::Names::None;
		}

		return ret;
	},

	// z = 1
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::OUT;
		ret.dest = RegisterFile::Names::BC;
		ret.addr_dest = true;

		if (o.y != 6) {
			ret.source = rTable[o.y];
		}
		else {
			ret.source = RegisterFile::Names::None;
		}

		return ret;
	},

	// z = 2
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.dest = RegisterFile::Names::HL;
		ret.source = rpTable[o.p];

		if (o.q == 0) {
			ret.name = Instruction::Names::SBC;
		}
		else {
			ret.name = Instruction::Names::ADC;
		}

		return ret;
	},

	// z = 3
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::LD;

		if (o.q == 0) {
			ret.dest = RegisterFile::Names::Immediate;
			ret.addr_dest = true;
			ret.immediate_bytes = 2;
			ret.source = rpTable[o.p];
		}
		else {
			ret.dest = rpTable[o.p];
			ret.source = RegisterFile::Names::Immediate;
			ret.addr_source = true;
			ret.immediate_bytes = 2;
		}

		return ret;
	},

	// z = 4
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::NEG;

		return ret;
	},

	// z = 5
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		if (o.y != 1) {
			ret.name = Instruction::Names::RETN;
		}
		else {
			ret.name = Instruction::Names::RETI;
		}

		return ret;
	},

	// z = 6
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::IM;
		ret.dest = RegisterFile::Names::Immediate;
		ret.immediate_value = o.y;

		return ret;
	},

	// z = 7
	[](Opcode o) -> InstructionDetails {
		InstructionDetails ret;

		ret.name = Instruction::Names::LD;

		switch (o.y) {
		case 0:
			ret.dest = RegisterFile::Names::I;
			ret.source = RegisterFile::Names::A;
			break;
		case 1:
			ret.dest = RegisterFile::Names::R;
			ret.source = RegisterFile::Names::A;
			break;
		case 2:
			ret.dest = RegisterFile::Names::A;
			ret.source = RegisterFile::Names::I;
			break;
		case 3:
			ret.dest = RegisterFile::Names::A;
			ret.source = RegisterFile::Names::R;
			break;
		case 4:
			ret.name = Instruction::Names::RRD;
			break;
		case 5:
			ret.name = Instruction::Names::RLD;
			break;
		case 6:
		case 7:
			ret.name = Instruction::Names::NOP;
			break;
		}

		return ret;
	}
};

std::optional<Instruction> Decoder::decode(uint8_t b) {

	if (alt_op) {
		std::optional<Instruction> ret = decode_CB(b);

		if (ret) {
			Opcode op = parse_opcode(b);

			bool is_alt = op.z != 6;

			if (is_alt) {
				ret->name = get_alt_CB_name(ret->name);
				ret->source = alt_reg;
				ret->dest = rTable[op.z];
			}
			else {
				ret->dest = alt_reg;
			}

			alt_reg = RegisterFile::Names::None;
			ret->displacement = instruction_progress.displacement;
			instruction_progress = {};
		}

		alt_op = false;

		return ret;
	}

	if (needs_displacement) {
		instruction_progress.displacement = (int8_t)(0xFF) & b;

		needs_displacement = false;

		if (immediate_bytes == 0) {
			Instruction ret = instruction_progress;

			instruction_progress = Instruction{};

			return ret;
		}

		return std::nullopt;
	}

	if (immediate_bytes == 2) {
		instruction_progress.imm_low = b;

		immediate_bytes--;
		needs_high_imm = true;

		return std::nullopt;
	}

	if (immediate_bytes == 1) {
		if (needs_high_imm) {
			instruction_progress.imm_high = b;
		}
		else {
			instruction_progress.imm_low = b;
		}

		immediate_bytes--;
		needs_high_imm = false;

		Instruction ret = instruction_progress;

		instruction_progress = Instruction{};

		return ret;
	}

	if (prefix == 0) {
		switch (b) {
		case 0xCB:
		case 0xDD:
		case 0xED:
		case 0xFD:

			prefix = b;

			return std::nullopt;
		}
	}

	switch (prefix) {
	case 0xCB:
		return decode_CB(b);
	case 0xDD:
		return decode_DDFD(b);
	case 0xED:
		return decode_ED(b);
	case 0xFD:
		return decode_DDFD(b);
	case 0xDDCB:
		return decode_alt_prefix(b);
	case 0xFDCB:
		return decode_alt_prefix(b);
	}

	Opcode op = parse_opcode(b);

	Instruction ret;

	InstructionDetails details;

	if (op.x == 0) {
		details = NoPreX0ZTable[op.z](op);
	}
	else if (op.x == 1) {
		if (op.z != 6) {
			details.name = Instruction::Names::LD;
			details.dest = rTable[op.y];
			details.source = rTable[op.z];
		}
		else {
			if (op.y == 6) {
				details.name = Instruction::Names::HALT;
			}
		}
	}
	else if (op.x == 2) {
		details.name = aluTable[op.y];
		details.dest = RegisterFile::Names::A;
		details.source = rTable[op.z];
	}
	else if (op.x == 3) {
		details = NoPreX3ZTable[op.z](op);

		if (details.immediate_value > 0) {
			ret.imm = details.immediate_value;
		}
	}

	ret.name = details.name;
	ret.dest = details.alt_dest ? get_alt_name(details.dest) : details.dest;
	ret.addr_dest = details.addr_dest;
	ret.source = details.alt_source ? get_alt_name(details.source) : details.source;
	ret.addr_source = details.addr_source;
	ret.condition = details.condition;

	needs_displacement = details.needs_displacement;
	immediate_bytes = details.immediate_bytes;

	if (needs_displacement || immediate_bytes > 0) {
		instruction_progress = ret;

		return std::nullopt;
	}

	return ret;
}

std::optional<Instruction> Decoder::decode_CB(uint8_t b) {
	Opcode op = parse_opcode(b);

	Instruction ret;

	switch (op.x) {
	case 0:
		ret.name = rotTable[op.y];
		ret.dest = rTable[op.z];
		break;
	case 1:
		ret.name = Instruction::Names::BIT;
		ret.dest = RegisterFile::Names::Immediate;
		ret.imm = op.y;
		ret.source = rTable[op.z];
		break;
	case 2:
		ret.name = Instruction::Names::RES;
		ret.dest = RegisterFile::Names::Immediate;
		ret.imm = op.y;
		ret.source = rTable[op.z];
		break;
	case 3:
		ret.name = Instruction::Names::SET;
		ret.dest = RegisterFile::Names::Immediate;
		ret.imm = op.y;
		ret.source = rTable[op.z];
		break;
	}

	prefix = 0;

	return ret;
}

std::optional<Instruction> Decoder::decode_ED(uint8_t b) {

	Opcode op = parse_opcode(b);

	Instruction ret;

	InstructionDetails details;

	if (op.x == 1) {
		details = EDX1ZTable[op.z](op);
	}
	else if (op.x == 2) {
		if (op.z <= 3 && op.y >= 4) {
			uint8_t a = op.y - 4;
			uint8_t b = op.z;

			details.name = bliTable[a][b];
		}
		else {
			details.name = Instruction::Names::NONI;
		}
	}
	else {
		details.name = Instruction::Names::NONI;
	}

	ret.name = details.name;
	ret.dest = details.alt_dest ? get_alt_name(details.dest) : details.dest;
	ret.addr_dest = details.addr_dest;
	ret.source = details.alt_source ? get_alt_name(details.source) : details.source;
	ret.addr_source = details.addr_source;
	ret.condition = details.condition;

	needs_displacement = details.needs_displacement;
	immediate_bytes = details.immediate_bytes;

	if (immediate_bytes == 0) {
		ret.imm = details.immediate_value;
	}

	prefix = 0;

	if (needs_displacement || immediate_bytes > 0) {
		instruction_progress = ret;

		return std::nullopt;
	}

	return ret;
}

std::optional<Instruction> Decoder::decode_DDFD(uint8_t b) {

	switch (b) {
	case 0xDD:
	case 0xED:
	case 0xFD:
	{
		Instruction ret = Instruction{};

		ret.name = Instruction::Names::NONI;

		return ret;
	}

	case 0xCB:
		prefix = 0xCB || (prefix << 8);

		return std::nullopt;
	}

	RegisterFile::Names full = RegisterFile::Names::IX;
	RegisterFile::Names high = RegisterFile::Names::IXH;
	RegisterFile::Names low = RegisterFile::Names::IXL;

	if (prefix == 0xFD) {
		full = RegisterFile::Names::IY;
		high = RegisterFile::Names::IYH;
		low = RegisterFile::Names::IXL;
	}

	prefix = 0;

	std::optional<Instruction> ret = decode(b);

	if (!ret) {
		if (instruction_progress.source == RegisterFile::Names::HL && instruction_progress.addr_source) {
			instruction_progress.source = full;

			needs_displacement = true;

			return std::nullopt;
		}
		else if (instruction_progress.dest == RegisterFile::Names::HL && instruction_progress.addr_dest) {
			instruction_progress.dest = full;

			needs_displacement = true;

			return std::nullopt;
		}
		else {
			if (instruction_progress.source == RegisterFile::Names::HL) {
				if (instruction_progress.name != Instruction::Names::EX && instruction_progress.dest != RegisterFile::Names::DE) {
					instruction_progress.source = full;
				}
			}

			if (instruction_progress.dest == RegisterFile::Names::HL) {
				instruction_progress.dest = full;
			}

			if (instruction_progress.source == RegisterFile::Names::H) {
				instruction_progress.source = high;
			}

			if (instruction_progress.dest == RegisterFile::Names::H) {
				instruction_progress.dest = high;
			}

			if (instruction_progress.source == RegisterFile::Names::L) {
				instruction_progress.source = low;
			}

			if (instruction_progress.dest == RegisterFile::Names::L) {
				instruction_progress.dest = low;
			}
		}
	}

	return ret;
}

std::optional<Instruction> Decoder::decode_alt_prefix(uint8_t b) {

	alt_reg = RegisterFile::Names::IX;

	if (prefix == 0xFDCB) {
		alt_reg = RegisterFile::Names::IY;
	}

	needs_displacement = true;

	// This stops decode from returning early with garbage data
	immediate_bytes = 40;

	std::optional<Instruction> ret = decode(b);

	immediate_bytes = 0;

	alt_op = true;

	return std::nullopt;

}

