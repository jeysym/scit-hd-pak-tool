#pragma once

#include <cstdint>
#include <bit>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <cstdlib>
#include <algorithm>


// Takes pointer to 32-bit big-endian value and makes it the native endianess (little-endian on amd64).
inline void BE_to_native(uint32_t* value)
{
	if constexpr (std::endian::native == std::endian::little) {
		char* valueBytes = reinterpret_cast<char*>(value);
		std::swap(valueBytes[0], valueBytes[3]);
		std::swap(valueBytes[1], valueBytes[2]);
	}
}

// Takes pointer to 64-bit big-endian value and makes it the native endianess (little-endian on amd64).
inline void BE_to_native(uint64_t* value)
{
	if constexpr (std::endian::native == std::endian::little) {
		char* valueBytes = reinterpret_cast<char*>(value);
		std::swap(valueBytes[0], valueBytes[7]);
		std::swap(valueBytes[1], valueBytes[6]);
		std::swap(valueBytes[2], valueBytes[5]);
		std::swap(valueBytes[3], valueBytes[4]);
	}
}

// Gets 32-bit unsigned integer from memory and advances the pointer.
inline uint32_t get_uint32(const char*& memory) {
	uint32_t result = *(reinterpret_cast<const uint32_t*>(memory));
	BE_to_native(&result);
	memory += sizeof(uint32_t);
	return result;
}

// Gets 64-bit unsigned integer from memory and advances the pointer.
inline uint64_t get_uint64(const char*& memory) {
	uint64_t result = *(reinterpret_cast<const uint64_t*>(memory));
	BE_to_native(&result);
	memory += sizeof(uint64_t);
	return result;
}

// Gets UTF8 string from memory and advances the pointer. String must be zero terminated.
inline std::string get_string(const char*& memory) {
	std::string result{ memory };
	memory += (result.size() + 1);
	return result;
}

inline void set_uint32(char*& memory, uint32_t value) {
	uint32_t* target = reinterpret_cast<uint32_t*>(memory);
	*target = value;
	BE_to_native(target);
	memory += sizeof(uint32_t);
}

inline void set_uint64(char*& memory, uint64_t value) {
	uint64_t* target = reinterpret_cast<uint64_t*>(memory);
	*target = value;
	BE_to_native(target);
	memory += sizeof(uint64_t);
}

inline void set_string(char*& memory, std::string value) {
	std::memcpy(memory, value.c_str(), value.size() + 1);
	memory += (value.size() + 1);
}

enum Error {
	SUCCESS = 0,
	MEMORY_ERROR,
	IO_ERROR,
	ARGUMENT_ERROR
};