#pragma once

#include "soft80.h"
#include "interruptingdevice.h"

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

class NMITerminalDevice : public InterruptingDevice {

public:

	uint8_t read(uint8_t port_lo, uint8_t port_hi) override {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) override {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

private:

	void executor() override {
		if (should_read) {

			std::getline(std::cin, in_buffer);

			needs_len = true;

			zcpu->signal_nmi();

			should_read = false;
		}
	}

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};

class INT0TerminalDevice : public InterruptingDevice {

public:

	uint8_t read(uint8_t port_lo, uint8_t port_hi) override {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) override {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

private:

	void executor() override {
		if (should_read) {

			std::getline(std::cin, in_buffer);

			needs_len = true;

			zcpu->signal_int();

			wait_cpu_int_ack();

			zcpu->data_bus = 0xF7;

			should_read = false;
		}
	}

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};

class INT1TerminalDevice : public InterruptingDevice {

public:

	uint8_t read(uint8_t port_lo, uint8_t port_hi) override {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) override {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

private:

	void executor() override {
		if (should_read) {

			std::getline(std::cin, in_buffer);

			needs_len = true;

			zcpu->signal_int();

			should_read = false;
		}
	}

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};

class INT2TerminalDevice : public InterruptingDevice {

public:

	uint8_t read(uint8_t port_lo, uint8_t port_hi) override {
		if (needs_len) {
			needs_len = false;

			return in_buffer.size();
		}
		else {
			size_t idx = in_buffer.size() - port_hi;

			return in_buffer[idx];
		}
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) override {
		if (b == 0xFF) {
			should_read = true;
		}
		else {
			std::cout << b;
		}
	}

private:

	void executor() override {
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
	}

	std::atomic<bool> should_read{ false };
	std::atomic<bool> needs_len{ false };
	std::string in_buffer;

};
