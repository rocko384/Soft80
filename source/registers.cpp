#include "registers.h"

std::optional<uint16_t> RegisterFile::get_value(Names name) {
	
	switch (name) {
	case Names::A:
		return main.A;
	case Names::B:
		return main.B;
	case Names::C:
		return main.C;
	case Names::D:
		return main.D;
	case Names::E:
		return main.E;
	case Names::F:
		return main.F;
	case Names::H:
		return main.H;
	case Names::L:
		return main.L;
	case Names::AF:
		return main.AF;
	case Names::BC:
		return main.BC;
	case Names::DE:
		return main.DE;
	case Names::HL:
		return main.HL;
	case Names::SP:
		return SP;
	case Names::Aalt:
		return alt.A;
	case Names::Balt:
		return alt.B;
	case Names::Calt:
		return alt.C;
	case Names::Dalt:
		return alt.D;
	case Names::Ealt:
		return alt.E;
	case Names::Falt:
		return alt.F;
	case Names::Halt:
		return alt.H;
	case Names::Lalt:
		return alt.L;
	case Names::AFalt:
		return alt.AF;
	case Names::BCalt:
		return alt.BC;
	case Names::DEalt:
		return alt.DE;
	case Names::HLalt:
		return alt.HL;
	case Names::IXH:
		return IXH;
	case Names::IXL:
		return IXL;
	case Names::IX:
		return IX;
	case Names::IYH:
		return IYH;
	case Names::IYL:
		return IYL;
	case Names::IY:
		return IY;
	case Names::I:
		return I;
	case Names::R:
		return R;
	case Names::Immediate:
	case Names::None:
		break;
	}

	return std::nullopt;
}

void RegisterFile::set_value(Names name, uint16_t value) {

	switch (name) {
	case Names::A:
		main.A = uint8_t(0xFF) & value;
		break;
	case Names::B:
		main.B = uint8_t(0xFF) & value;
		break;
	case Names::C:
		main.C = uint8_t(0xFF) & value;
		break;
	case Names::D:
		main.D = uint8_t(0xFF) & value;
		break;
	case Names::E:
		main.E = uint8_t(0xFF) & value;
		break;
	case Names::F:
		main.F = uint8_t(0xFF) & value;
		break;
	case Names::H:
		main.H = uint8_t(0xFF) & value;
		break;
	case Names::L:
		main.L = uint8_t(0xFF) & value;
		break;
	case Names::AF:
		main.AF = value;
		break;
	case Names::BC:
		main.BC = value;
		break;
	case Names::DE:
		main.DE = value;
		break;
	case Names::HL:
		main.HL = value;
		break;
	case Names::SP:
		SP = value;
		break;
	case Names::Aalt:
		alt.A = uint8_t(0xFF) & value;
		break;
	case Names::Balt:
		alt.B = uint8_t(0xFF) & value;
		break;
	case Names::Calt:
		alt.C = uint8_t(0xFF) & value;
		break;
	case Names::Dalt:
		alt.D = uint8_t(0xFF) & value;
		break;
	case Names::Ealt:
		alt.E = uint8_t(0xFF) & value;
		break;
	case Names::Falt:
		alt.F = uint8_t(0xFF) & value;
		break;
	case Names::Halt:
		alt.H = uint8_t(0xFF) & value;
		break;
	case Names::Lalt:
		alt.L = uint8_t(0xFF) & value;
		break;
	case Names::AFalt:
		alt.AF = value;
		break;
	case Names::BCalt:
		alt.BC = value;
		break;
	case Names::DEalt:
		alt.DE = value;
		break;
	case Names::HLalt:
		alt.HL = value;
		break;
	case Names::IXH:
		IXH = uint8_t(0xFF) & value;
		break;
	case Names::IXL:
		IXL = uint8_t(0xFF) & value;
		break;
	case Names::IX:
		IX = value;
		break;
	case Names::IYH:
		IYH = uint8_t(0xFF) & value;
		break;
	case Names::IYL:
		IYL = uint8_t(0xFF) & value;
		break;
	case Names::IY:
		IY = value;
		break;
	case Names::I:
		I = uint8_t(0xFF) & value;
		break;
	case Names::R:
		R = uint8_t(0xFF) & value;
		break;
	case Names::Immediate:
	case Names::None:
		break;
	}

}


bool RegisterFile::is_16bit(Names name) {

	switch (name) {
	case Names::A:
	case Names::B:
	case Names::C:
	case Names::D:
	case Names::E:
	case Names::F:
	case Names::H:
	case Names::L:
	case Names::Aalt:
	case Names::Balt:
	case Names::Calt:
	case Names::Dalt:
	case Names::Ealt:
	case Names::Falt:
	case Names::Halt:
	case Names::Lalt:
	case Names::IXH:
	case Names::IXL:
	case Names::IYH:
	case Names::IYL:
	case Names::I:
	case Names::R:
	case Names::Immediate:
	case Names::None:
		return false;
		break;

	case Names::AF:
	case Names::BC:
	case Names::DE:
	case Names::HL:
	case Names::SP:
	case Names::AFalt:
	case Names::BCalt:
	case Names::DEalt:
	case Names::HLalt:
	case Names::IX:
	case Names::IY:
		return true;
	}

	return false;
}
