#pragma once

#include <cstdint>
#include <iostream>

class TerminalDevice {

public:

	uint8_t read(uint8_t port_lo, uint8_t port_hi) {
		return 0;
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		std::cout << b;
	}

};