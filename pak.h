#pragma once

#include "common.h"

struct File {
	uint32_t name_length;	// 32-bit   - Length of the name string.
	uint64_t size;			// 64-bit   - Size of the file data.
	uint64_t pak_offset;	// 64-bit   - Offset from the beginning of the pak to file data.
	uint32_t crc32;			// 32-bit   - CRC32.
	uint32_t zero = 0;		// 32-bit   - Always zero, probably padding.
	std::string name;		// 0-terminated UTF8 string - Name of the file.
							// Example: "campfire_01.fxt\0"

	char* data = nullptr;

	static File load_from_memory(const char*& data);
	void save_to_memory(char*& data) const;
	size_t size_in_pak() const;
};

struct Dir {
	uint32_t dir_index;		// 32-bit   - Directory index (1-based).
	uint32_t name_length;	// 32-bit   - Length of the name string.
	uint32_t num_of_files;	// 32-bit   - Number of files in directory.
	uint32_t zero = 0;		// 32-bit   - Always zero, probably some padding.
	std::string name;		// 0-terminated UTF8 string - Name of the directory.
							// Example: "\\bin_win32\\\\effects\\\0"

	std::vector<File> files;

	static Dir load_from_memory(const char*& data);
	void save_to_memory(char*& data) const;
	size_t size_in_pak() const;
	std::filesystem::path get_dir_path() const;
	void set_dir_path(std::filesystem::path dir_path);
};

struct Pak {
	static constexpr uint64_t TAG_MAGIC = 0x504B424500000001;

	uint64_t tag;			// 64-bit    - Tag.
							// Usually is 50 4B 42 45 00 00 00 01 - PKBE 0001.
							// Which probably means: Pak Big Endian Version 1.
	uint64_t base_offset;	// 64-bit BE - Base offset of data in file.
	uint32_t num_dirs;		// 64-bit BE - Number of directories in pak.
	uint32_t zero;			// 32-bit    - Always zero, probably some padding.

	std::vector<Dir> directories;

	static Pak load_from_file(std::filesystem::path file_path);
	static Pak load_from_memory(const char* data);
	static Pak load_from_dir(std::filesystem::path dir_path);
	void save_to_file(std::filesystem::path file_path) const;
	void save_to_memory(char* data) const;
	void save_to_dir(std::filesystem::path dir_path) const;

	size_t size_in_pak() const;
};