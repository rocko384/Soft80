#pragma once

#include "soft80.h"

class InterruptingDevice {

public:

	InterruptingDevice() {
		execution_thread = std::thread(&InterruptingDevice::executor_impl, this);
	}

	~InterruptingDevice() {
		should_executor_exit = true;
		execution_thread.join();
	}

	virtual uint8_t read(uint8_t port_lo, uint8_t port_hi) = 0;

	virtual void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) = 0;

	void connect(Soft80& zcpu) {
		this->zcpu = &zcpu;
	}

	void cycle_clock() {
		should_cycle = true;
	}

protected:

	std::atomic<bool> should_cycle{ false };
	std::atomic<bool> should_executor_exit{ false };

	virtual void executor() = 0;

	void executor_impl() {
		while (!should_executor_exit) {
			executor();

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

};
