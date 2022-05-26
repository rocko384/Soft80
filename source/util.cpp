#include "util.h"

#include <fstream>


std::vector<uint8_t> load_bin_file(const std::filesystem::path& path) {

	std::vector<uint8_t> ret;

	size_t file_size = std::filesystem::file_size(path);

	ret.reserve(file_size);

	std::ifstream file(path, std::ios::binary);

	while (!file.eof()) {
		char byte;
		file.get(byte);
		ret.push_back(static_cast<uint8_t>(byte));
	}

	return ret;
}
