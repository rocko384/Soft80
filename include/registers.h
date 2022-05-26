#pragma once

#include <optional>
#include <cstdint>

enum class FlagNames {
	None			= 0,
	Sign			= 1 << 0,
	SignSet			= 1 << 1,
	SignReset		= 1 << 2,
	Zero			= 1 << 3,
	ZeroSet			= 1 << 4,
	ZeroReset		= 1 << 5,
	F5				= 1 << 6,
	HalfCarry		= 1 << 7,
	HalfCarrySet	= 1 << 8,
	HalfCarryReset	= 1 << 9,
	F3				= 1 << 10,
	Parity			= 1 << 11,
	Overflow		= 1 << 12,
	OverflowSet		= 1 << 13,
	OverflowReset	= 1 << 14,
	Subtract		= 1 << 15,
	SubtractSet		= 1 << 16,
	SubtractReset	= 1 << 17,
	Carry			= 1 << 18,
	CarrySet		= 1 << 19,
	CarryReset		= 1 << 20
};

inline constexpr FlagNames operator &(FlagNames a, FlagNames b) {
	return static_cast<FlagNames>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr FlagNames operator |(FlagNames a, FlagNames b) {
	return static_cast<FlagNames>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr FlagNames operator ^(FlagNames a, FlagNames b) {
	return static_cast<FlagNames>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

inline constexpr FlagNames operator ~(FlagNames a) {
	return static_cast<FlagNames>(~static_cast<uint32_t>(a));
}

inline constexpr FlagNames& operator &=(FlagNames& a, FlagNames b) {
	a = a & b;
	return a;
}

inline constexpr FlagNames& operator |=(FlagNames& a, FlagNames b) {
	a = a | b;
	return a;
}

inline constexpr FlagNames& operator ^=(FlagNames& a, FlagNames b) {
	a = a ^ b;
	return a;
}

inline constexpr bool is_flag_set(FlagNames field, FlagNames flag) {
	return (field & flag) != FlagNames::None;
}

namespace FlagBits {
	static const uint8_t Sign		= 0b1000'0000;
	static const uint8_t Zero		= 0b0100'0000;
	static const uint8_t F5			= 0b0010'0000;
	static const uint8_t HalfCarry	= 0b0001'0000;
	static const uint8_t F3			= 0b0000'1000;
	static const uint8_t Overflow	= 0b0000'0100;
	static const uint8_t Subtract	= 0b0000'0010;
	static const uint8_t Carry		= 0b0000'0001;
};

inline constexpr uint8_t set_flag_bit(uint8_t flags, uint8_t flag_bit) {
	return flags | flag_bit;
}

inline constexpr uint8_t reset_flag_bit(uint8_t flags, uint8_t flag_bit) {
	uint8_t t = flags | flag_bit;
	return t ^ flag_bit;
}

struct Flags {
	
	bool sign;
	bool zero;
	bool half_carry;
	
	union {
		bool parity;
		bool overflow;
	};

	bool subtract;
	bool carry;

};

constexpr Flags parse_flags(uint8_t F) {
	Flags ret{ 0 };

	ret.sign		= F & 0b10000000;
	ret.zero		= F & 0b01000000;
	ret.half_carry	= F & 0b00010000;
	ret.parity		= F & 0b00000100;
	ret.subtract	= F & 0b00000010;
	ret.carry		= F & 0b00000001;

	return ret;
}

struct GeneralRegisters {

	union {
		uint16_t AF{ 0 };

		struct {
			uint8_t F;
			uint8_t A;
		};
	};

	union {
		uint16_t BC{ 0 };

		struct {
			uint8_t C;
			uint8_t B;
		};
	};

	union {
		uint16_t DE{ 0 };

		struct {
			uint8_t E;
			uint8_t D;
		};
	};

	union {
		uint16_t HL{ 0 };

		struct {
			uint8_t L;
			uint8_t H;
		};
	};

};

struct RegisterFile {

	enum class Names {
		A, B, C, D, E, F, H, L,
		AF, BC, DE, HL, SP,
		Aalt, Balt, Calt, Dalt, Ealt, Falt, Halt, Lalt,
		AFalt, BCalt, DEalt, HLalt,
		IXH, IXL, IX, IYH, IYL, IY, I, R,
		Immediate, None
	};

	GeneralRegisters main;
	GeneralRegisters alt;

	union {
		uint16_t IX{ 0 };

		struct {
			uint8_t IXL;
			uint8_t IXH;
		};
	};

	union {
		uint16_t IY{ 0 };

		struct {
			uint8_t IYL;
			uint8_t IYH;
		};
	};

	uint16_t SP{ 0 };

	uint8_t I{ 0 };
	uint8_t R{ 0 };

	uint16_t PC{ 0 };

	std::optional<uint16_t> get_value(Names name);
	void set_value(Names name, uint16_t value);

	static bool is_16bit(Names name);

};
