#pragma once

#include "memorymap.h"

#include <filesystem>
#include <vector>
#include <algorithm>
#include <cstdint>

std::vector<uint8_t> load_bin_file(const std::filesystem::path& path);

template<size_t Size>
ROM<Size> load_ROM_from_file(const std::filesystem::path& path) {
	std::vector<uint8_t> data = load_bin_file(path);

	ROM<Size> ret;

	std::copy(data.begin(), data.end(), ret.data.begin());

	return ret;
}
