#pragma once

#include "registers.h"
#include "decoder.h"
#include "memorymap.h"
#include "devicemap.h"

#include <optional>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <cstdint>

class Soft80 {

public:

	Soft80();
	~Soft80();

	bool read_busack();
	bool read_halt();
	bool read_iorq();
	bool read_m1();
	bool read_mreq();
	bool read_rd();
	bool read_wr();
	bool read_rfsh();

	void set_busreq_source(bool& b);
	void set_int_source(bool& b);
	void set_nmi_source(bool& b);
	void set_reset_source(bool& b);
	void set_wait_source(bool& b);

	void cycle_clock();

	void kill();

	MemoryMap memory;
	DeviceMap devices;

private:

	std::atomic<bool> should_cycle{ false };
	std::atomic<bool> should_executor_exit{ false };

	void executor();
	void wait_next_clock();

	void fetch_opcode();
	uint8_t read_memory(uint16_t address);
	void write_memory(uint16_t address, uint8_t value);
	uint8_t read_io(uint8_t port_lo, uint8_t port_hi);
	void write_io(uint8_t port_lo, uint8_t port_hi, uint8_t value);
	void bus_acknowledge();
	void int_acknowledge();
	void nmi_acknowledge();

	std::thread execution_thread;

	void execute_instruction();

	RegisterFile registers;
	Decoder decoder;

	std::optional<Instruction> current_instruction{ std::nullopt };

	size_t total_t_cycles{ 0 };
	size_t current_t_cycles{ 0 };

	enum class M_Cycles {
		OpcodeFetch,
		MemRead,
		MemWrite,
		IORead,
		IOWrite,
		IntAck,
		Other
	};

	M_Cycles current_m_cycle{ M_Cycles::OpcodeFetch };

	uint8_t data_bus;
	uint16_t address_bus;

	bool busack{ false };
	bool halt{ false };
	bool iorq{ false };
	bool m1{ false };
	bool mreq{ false };
	bool rd{ false };
	bool wr{ false };
	bool rfsh{ false };

	bool iff1{ false };
	bool iff2{ false };

	size_t interrupt_mode{ 0 };

	bool* busreq_source{ nullptr };
	bool* int_source{ nullptr };
	bool* nmi_source{ nullptr };
	bool* reset_source{ nullptr };
	bool* wait_source{ nullptr };

};