#pragma once

#include <cstdint>
#include <functional>
#include <array>
#include <vector>

template <typename T>
concept DeviceType =
	requires(T r, uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		{ r.read(port_lo, port_hi) } -> std::same_as<uint8_t>;
		r.write(port_lo, port_hi, b);
	};

struct DeviceMap {

	struct Mapping {

		template <DeviceType T>
		Mapping(T& device, uint8_t port) {
			read_fn = [&device](uint8_t port_lo, uint8_t port_hi) -> uint8_t {
				return device.read(port_lo, port_hi);
			};

			write_fn = [&device](uint8_t port_lo, uint8_t port_hi, uint8_t b) -> void {
				device.write(port_lo, port_hi, b);
			};

			this->port = port;
		}

		std::function<uint8_t(uint8_t, uint8_t)> read_fn;
		std::function<void(uint8_t, uint8_t, uint8_t)> write_fn;

		uint8_t port;
	};

	enum class Error {
		OK,
		Port_In_Use
	};

	Error add_mapping(Mapping m) {

		for (auto& existing : mappings) {
			if (m.port == existing.port) {
				return Error::Port_In_Use;
			}
		}

		mappings.push_back(m);

		return Error::OK;
	}

	template <DeviceType T>
	Error add_mapping(T& device, uint8_t port) {
		return add_mapping(Mapping(device, port));
	}

	uint8_t read(uint8_t port_lo, uint8_t port_hi) {
		for (auto& mapping : mappings) {
			if (port_lo == mapping.port) {
				return mapping.read_fn(port_lo, port_hi);
			}
		}

		return 0;
	}

	void write(uint8_t port_lo, uint8_t port_hi, uint8_t b) {
		for (auto& mapping : mappings) {
			if (port_lo == mapping.port) {
				mapping.write_fn(port_lo, port_hi, b);
			}
		}
	}

	std::vector<Mapping> mappings;

};