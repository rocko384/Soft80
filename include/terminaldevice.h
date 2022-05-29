#pragma once

#include "soft80.h"

#include <thread>
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

class NMITerminalDevice {

public:

	NMITerminalDevice() {
		execution_thread = std::thread(&NMITerminalDevice::executor, this);
	}

	~NMITerminalDevice() {
		should_executor_exit = true;
		execution_thread.join();
	}

	uint8_t read(uint8_t port_lo, uint8_t port_hi) {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

	void connect(Soft80& zcpu) {
		this->zcpu = &zcpu;
	}

	void cycle_clock() {
		should_cycle = true;
	}

private:

	std::atomic<bool> should_cycle{ false };
	std::atomic<bool> should_executor_exit{ false };

	void executor() {
		while (!should_executor_exit) {
			if (should_read) {

				std::getline(std::cin, in_buffer);

				needs_len = true;

				zcpu->signal_nmi();

				should_read = false;
			}

			std::this_thread::yield();
		}
	}

	void wait_next_clock() {
		while (!should_cycle) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}

		should_cycle = false;
	}

	std::thread execution_thread;

	Soft80* zcpu{ nullptr };

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};

class INT0TerminalDevice {

public:

	INT0TerminalDevice() {
		execution_thread = std::thread(&INT0TerminalDevice::executor, this);
	}

	~INT0TerminalDevice() {
		should_executor_exit = true;
		execution_thread.join();
	}

	uint8_t read(uint8_t port_lo, uint8_t port_hi) {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

	void connect(Soft80& zcpu) {
		this->zcpu = &zcpu;
	}

	void cycle_clock() {
		should_cycle = true;
	}

private:

	std::atomic<bool> should_cycle{ false };
	std::atomic<bool> should_executor_exit{ false };

	void executor() {
		while (!should_executor_exit) {
			if (should_read) {

				std::getline(std::cin, in_buffer);

				needs_len = true;

				zcpu->signal_int();

				wait_cpu_int_ack();

				zcpu->data_bus = 0xF7;

				should_read = false;
			}

			std::this_thread::yield();
		}
	}

	void wait_next_clock() {
		while (!should_cycle) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}

		should_cycle = false;
	}

	void wait_cpu_int_ack() {
		while (!(zcpu->read_iorq() && zcpu->read_m1())) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}
	}

	std::thread execution_thread;

	Soft80* zcpu{ nullptr };

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};

class INT1TerminalDevice {

public:

	INT1TerminalDevice() {
		execution_thread = std::thread(&INT1TerminalDevice::executor, this);
	}

	~INT1TerminalDevice() {
		should_executor_exit = true;
		execution_thread.join();
	}

	uint8_t read(uint8_t port_lo, uint8_t port_hi) {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

	void connect(Soft80& zcpu) {
		this->zcpu = &zcpu;
	}

	void cycle_clock() {
		should_cycle = true;
	}

private:

	std::atomic<bool> should_cycle{ false };
	std::atomic<bool> should_executor_exit{ false };

	void executor() {
		while (!should_executor_exit) {
			if (should_read) {

				std::getline(std::cin, in_buffer);

				needs_len = true;

				zcpu->signal_int();

				should_read = false;
			}

			std::this_thread::yield();
		}
	}

	void wait_next_clock() {
		while (!should_cycle) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}

		should_cycle = false;
	}

	std::thread execution_thread;

	Soft80* zcpu{ nullptr };

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};

class INT2TerminalDevice {

public:

	INT2TerminalDevice() {
		execution_thread = std::thread(&INT2TerminalDevice::executor, this);
	}

	~INT2TerminalDevice() {
		should_executor_exit = true;
		execution_thread.join();
	}

	uint8_t read(uint8_t port_lo, uint8_t port_hi) {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

	void connect(Soft80& zcpu) {
		this->zcpu = &zcpu;
	}

	void cycle_clock() {
		should_cycle = true;
	}

private:

	std::atomic<bool> should_cycle{ false };
	std::atomic<bool> should_executor_exit{ false };

	void executor() {
		while (!should_executor_exit) {
			if (should_read) {

				std::getline(std::cin, in_buffer);

				needs_len = true;

				zcpu->signal_int();
				wait_cpu_int_ack();
				zcpu->data_bus = 0;

				wait_cpu_halt();

				zcpu->signal_int();
				wait_cpu_int_ack();
				zcpu->data_bus = 2;

				should_read = false;
			}

			std::this_thread::yield();
		}
	}

	void wait_next_clock() {
		while (!should_cycle) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}

		should_cycle = false;
	}

	void wait_cpu_int_ack() {
		while (!(zcpu->read_iorq() && zcpu->read_m1())) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}
	}

	void wait_cpu_halt() {
		while (!zcpu->read_halt()) {
			if (should_executor_exit) {
				exit(0);
			}

			std::this_thread::yield();
		}
	}

	std::thread execution_thread;

	Soft80* zcpu{ nullptr };

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};
