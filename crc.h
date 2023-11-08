#pragma once
#include "common.h"

struct Crc32 {
	static constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320;
	static constexpr uint32_t CRC32_INITIAL = 0xFFFFFFFF;

	uint32_t table[256];

	Crc32();
	uint32_t calculate(const char* message, size_t message_size);
};
