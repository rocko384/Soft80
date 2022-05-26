#pragma once

#include <cstdint>
#include <functional>
#include <array>
#include <vector>

template <typename T>
concept MemoryRegionType =
	requires(T r, uint16_t addr, uint8_t b) {
		{ r.read(addr) } -> std::same_as<uint8_t>;
		r.write(addr, b);
	};

template <size_t Size, bool Mutable = true>
struct MemoryBlock {

	MemoryBlock() {
		memset(data.data(), 0, data.size());
	}

	uint8_t read(uint16_t addr) {
		return data[addr];
	}

	void write(uint16_t addr, uint8_t b) {
		if constexpr (Mutable) {
			data[addr] = b;
		}
	}

	std::array<uint8_t, Size> data;

};

template <size_t Size>
using ROM = MemoryBlock<Size, false>;

template <size_t Size>
using RAM = MemoryBlock<Size, true>;

struct MemoryMap {

	struct Mapping {

		template <MemoryRegionType T>
		Mapping(T& region, uint16_t low, uint16_t high) {
			read_fn = [&region, low](uint16_t addr) -> uint8_t {
				return region.read(addr - low);
			};

			write_fn = [&region, low](uint16_t addr, uint8_t b) -> void {
				region.write(addr - low, b);
			};

			low_bound = low;
			high_bound = high;
		}

		std::function<uint8_t(uint16_t)> read_fn;
		std::function<void(uint16_t, uint8_t)> write_fn;

		uint16_t low_bound;
		uint16_t high_bound;
	};

	enum class Error {
		OK,
		Overlaps_Existing
	};

	Error add_mapping(Mapping m) {

		for (auto& existing : mappings) {
			if ((m.low_bound <= existing.high_bound && m.low_bound >= existing.low_bound)
				|| (m.high_bound <= existing.high_bound && m.high_bound >= existing.low_bound)) {
				return Error::Overlaps_Existing;
			}
		}

		mappings.push_back(m);

		return Error::OK;
	}

	template <MemoryRegionType T>
	Error add_mapping(T& region, uint16_t low, uint16_t high) {
		return add_mapping(Mapping(region, low, high));
	}

	uint8_t read(uint16_t addr) {
		for (auto& mapping : mappings) {
			if (addr >= mapping.low_bound && addr <= mapping.high_bound) {
				return mapping.read_fn(addr);
			}
		}

		return 0;
	}

	void write(uint16_t addr, uint8_t b) {
		for (auto& mapping : mappings) {
			if (addr >= mapping.low_bound && addr <= mapping.high_bound) {
				mapping.write_fn(addr, b);
			}
		}
	}

	std::vector<Mapping> mappings;

};
