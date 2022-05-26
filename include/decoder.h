#pragma once

#include "registers.h"

#include <optional>
#include <cstdint>

struct Opcode {
	uint8_t x : 2;

	union {
		uint8_t y : 3;

		struct {
			uint8_t q : 1;
			uint8_t p : 2;
		};
	};

	uint8_t z : 3;
};

constexpr Opcode parse_opcode(uint8_t b) {
	Opcode ret{ 0 };

	ret.x = (b & 0b11000000) >> 6;
	ret.y = (b & 0b00111000) >> 3;
	ret.z = (b & 0b00000111);

	return ret;
}

struct Instruction {
	enum class Names {
		ADC, ADD, AND, BIT, CALL, CCF, CP, CPD, CPDR, CPI, CPIR, CPL, DAA,
		DEC, DI, DJNZ, EI, EX, EXX, HALT, IM, IN, INC, IND, INDR, INI, INIR,
		JP, JR, LD, LDD, LDDR, LDI, LDIR, NEG, NOP, OR, OTDR, OTIR, OUT, OUTD,
		OUTI, POP, PUSH, RES, RET, RETI, RETN, RL, RLA, RLC, RLCA, RLD, RR,
		RRA, RRC, RRCA, RRD, RST, SBC, SCF, SET, SLA, SRA, SLL, SRL, SUB, XOR,

		// Special case instructions
		NONI, RLCalt, RRCalt, RLalt, RRalt, SLAalt, SRAalt, SLLalt, SRLalt,
		RESalt, SETalt
	};

	enum class Conditions {
		NZ, Z, NC, C, PO, PE, P, M, None
	};

	Names name{ Instruction::Names::NOP };
	RegisterFile::Names dest{ RegisterFile::Names::None };
	bool addr_dest{ false };
	RegisterFile::Names source{ RegisterFile::Names::None };
	bool addr_source{ false };
	Conditions condition{ Conditions::None };

	int8_t displacement{ 0 };

	union {
		struct {
			uint8_t imm_low;
			uint8_t imm_high;
		};

		uint16_t imm{ 0 };
	};

};

constexpr RegisterFile::Names get_alt_name(RegisterFile::Names r) {

	switch (r) {
	case RegisterFile::Names::A: return RegisterFile::Names::Aalt;
	case RegisterFile::Names::B: return RegisterFile::Names::Balt;
	case RegisterFile::Names::C: return RegisterFile::Names::Calt;
	case RegisterFile::Names::D: return RegisterFile::Names::Dalt;
	case RegisterFile::Names::E: return RegisterFile::Names::Ealt;
	case RegisterFile::Names::F: return RegisterFile::Names::Falt;
	case RegisterFile::Names::H: return RegisterFile::Names::Halt;
	case RegisterFile::Names::L: return RegisterFile::Names::Lalt;
	case RegisterFile::Names::AF: return RegisterFile::Names::AFalt;
	case RegisterFile::Names::BC: return RegisterFile::Names::BCalt;
	case RegisterFile::Names::DE: return RegisterFile::Names::DEalt;
	case RegisterFile::Names::HL: return RegisterFile::Names::HLalt;
	}

	return RegisterFile::Names::Immediate;
}

constexpr Instruction::Names get_alt_CB_name(Instruction::Names i) {

	switch (i) {
	case Instruction::Names::RLC: return Instruction::Names::RLCalt;
	case Instruction::Names::RRC: return Instruction::Names::RRCalt;
	case Instruction::Names::RL: return Instruction::Names::RLalt;
	case Instruction::Names::RR: return Instruction::Names::RRalt;
	case Instruction::Names::SLA: return Instruction::Names::SLAalt;
	case Instruction::Names::SRA: return Instruction::Names::SRAalt;
	case Instruction::Names::SLL: return Instruction::Names::SLLalt;
	case Instruction::Names::SRL: return Instruction::Names::SRLalt;
	case Instruction::Names::BIT: return Instruction::Names::BIT;
	case Instruction::Names::RES: return Instruction::Names::RESalt;
	case Instruction::Names::SET: return Instruction::Names::SETalt;
	}

	return i;
}

const RegisterFile::Names rTable[]{
	RegisterFile::Names::B,
	RegisterFile::Names::C,
	RegisterFile::Names::D,
	RegisterFile::Names::E,
	RegisterFile::Names::H,
	RegisterFile::Names::L,
	RegisterFile::Names::HL, // Address in HL
	RegisterFile::Names::A
};

const RegisterFile::Names rpTable[]{
	RegisterFile::Names::BC,
	RegisterFile::Names::DE,
	RegisterFile::Names::HL,
	RegisterFile::Names::SP
};

const RegisterFile::Names rp2Table[]{
	RegisterFile::Names::BC,
	RegisterFile::Names::DE,
	RegisterFile::Names::HL,
	RegisterFile::Names::AF
};

const Instruction::Conditions ccTable[]{
	Instruction::Conditions::NZ,
	Instruction::Conditions::Z,
	Instruction::Conditions::NC,
	Instruction::Conditions::C,
	Instruction::Conditions::PO,
	Instruction::Conditions::PE,
	Instruction::Conditions::P,
	Instruction::Conditions::M
};

const Instruction::Names aluTable[]{
	Instruction::Names::ADD,
	Instruction::Names::ADC,
	Instruction::Names::SUB,
	Instruction::Names::SBC,
	Instruction::Names::AND,
	Instruction::Names::XOR,
	Instruction::Names::OR,
	Instruction::Names::CP
};

const Instruction::Names rotTable[]{
	Instruction::Names::RLC,
	Instruction::Names::RRC,
	Instruction::Names::RL,
	Instruction::Names::RR,
	Instruction::Names::SLA,
	Instruction::Names::SRA,
	Instruction::Names::SLL,
	Instruction::Names::SRL
};

const Instruction::Names bliTable[4][4]{
	{ Instruction::Names::LDI, Instruction::Names::CPI, Instruction::Names::INI, Instruction::Names::OUTI },
	{ Instruction::Names::LDD, Instruction::Names::CPD, Instruction::Names::IND, Instruction::Names::OUTD },
	{ Instruction::Names::LDIR, Instruction::Names::CPIR, Instruction::Names::INIR, Instruction::Names::OTIR },
	{ Instruction::Names::LDDR, Instruction::Names::CPDR, Instruction::Names::INDR, Instruction::Names::OTDR }
};

class Decoder {

public:

	std::optional<Instruction> decode(uint8_t b);

private:

	std::optional<Instruction> decode_CB(uint8_t b);
	std::optional<Instruction> decode_ED(uint8_t b);
	std::optional<Instruction> decode_DDFD(uint8_t b);
	std::optional<Instruction> decode_alt_prefix(uint8_t b);

	uint16_t prefix{ 0 };

	bool alt_op{ false };
	RegisterFile::Names alt_reg{ RegisterFile::Names::None };

	bool needs_displacement{ false };
	uint8_t immediate_bytes{ 0 };
	bool needs_high_imm{ false };

	Instruction instruction_progress;

};
