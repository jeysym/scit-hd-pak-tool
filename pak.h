#pragma once

#include "common.h"

struct File {
	uint32_t name_length;	// 32-bit   - Length of the name string.
	uint64_t size;			// 64-bit   - Size of the file data.
	uint64_t pak_offset;	// 64-bit   - Offset from the beginning of the pak to file data.
	uint32_t crc32;			// 32-bit   - CRC32.
	uint32_t zero;			// 32-bit   - Always zero, probably padding.
	std::string name;		// 0-terminated UTF8 string - Name of the file.

	void load(const char*& data);
};

struct Dir {
	uint32_t dir_index;		// 32-bit   - Directory index (1-based).
	uint32_t name_length;	// 32-bit   - Length of the name string.
	uint32_t num_of_files;	// 32-bit   - Number of files in directory.
	uint32_t zero;			// 32-bit   - Always zero, probably some padding.
	std::string name;		// 0-terminated UTF8 string - Name of the directory.

	std::vector<File> files;

	void load(const char*& data);
};

struct Pak {
	uint64_t tag;			// 64-bit    - Tag.
							// Usually is 50 4B 42 45 00 00 00 01 - PKBE 0001.
							// Which probably means: Pak Big Endian Version 1.
	uint64_t base_offset;	// 64-bit BE - Base offset of data in file.
	uint32_t num_dirs;		// 64-bit BE - Number of directories in pak.
	uint32_t zero;			// 32-bit    - Always zero, probably some padding.

	std::vector<Dir> directories;

	static Pak load_from_pak(const char* data);
	static Pak load_from_dir(const char* dir_path);
	void save_to_pak(const char* buffer);
	void save_to_dir(const char* dir_path);
};