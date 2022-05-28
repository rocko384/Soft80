#include "soft80.h"
#include "terminaldevice.h"
#include "util.h"

#include <iostream>
#include <chrono>

const size_t MHZ4 = 250;
const size_t HZ1 = 1000000000;

int main(int argc, char** argv) {

	MemoryMap map;

	ROM<8192> rom = load_ROM_from_file<8192>("tests/nmi_tty.bin");
	RAM<57344> ram;
	NMITerminalDevice term;

	Soft80 zcpu;

	zcpu.memory.add_mapping(rom, 0, 8191);
	zcpu.memory.add_mapping(ram, 8192, 65535);

	zcpu.devices.add_mapping(term, 80);

	term.connect(zcpu);

	auto start = std::chrono::steady_clock::now();

	while(true) {
		auto now = std::chrono::steady_clock::now();
		auto delta = now - start;

		if (delta.count() >= (MHZ4)) {
			start = now;
		
			zcpu.cycle_clock();
			term.cycle_clock();
		}
	}


	return 0;
}