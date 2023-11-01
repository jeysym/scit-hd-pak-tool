#include "pak.h"
#include <windows.h>
#include <iostream>

File File::load_from_memory(const char*& data)
{
	File result;

	// Parse the header
	result.name_length = get_uint32(data);
	result.size = get_uint64(data);
	result.pak_offset = get_uint64(data);
	result.crc32 = get_uint32(data);
	result.zero = get_uint32(data);
	assert(result.zero == 0);
	result.name = get_string(data);
	assert((result.name.size() + 1) == result.name_length);

	return result;
}

Dir Dir::load_from_memory(const char*& data)
{
	Dir result;

	// Parse the header
	result.dir_index = get_uint32(data);
	result.name_length = get_uint32(data);
	result.num_of_files = get_uint32(data);
	result.zero = get_uint32(data);
	assert(result.zero == 0);
	result.name = get_string(data);
	assert((result.name.size() + 1) == result.name_length);

	// Parse the files
	for (int file_idx = 0; file_idx < result.num_of_files; ++file_idx)
	{
		result.files.push_back(File::load_from_memory(data));
	}

	return result;
}

Pak Pak::load_from_memory(const char* data)
{
	Pak result;

	// Parse the header
	result.tag = get_uint64(data);
	result.base_offset = get_uint64(data);
	result.num_dirs = get_uint32(data);
	result.zero = get_uint32(data);
	assert(result.zero == 0);

	// Parse the directories
	for (int dir_idx = 0; dir_idx < result.num_dirs; ++dir_idx) {
		result.directories.push_back(Dir::load_from_memory(data));

		// Copy the actual file contents.
		Dir& current_dir = result.directories[dir_idx];
		for (int file_idx = 0; file_idx < current_dir.num_of_files; ++file_idx) {
			File& current_file = current_dir.files[file_idx];

			current_file.data = static_cast<char*>(std::malloc(current_file.size));
			std::memcpy(current_file.data, data + current_file.pak_offset, current_file.size);
		}
	}

	return result;
}

Pak Pak::load_from_dir(const char* dir_path)
{
	Pak result;
	// TODO(jeysym): Implement
	return result;
}

void Pak::save_to_file(const char* file_path)
{
	// TODO(jeysym): Implement
}

void Pak::save_to_dir(const char* dir_path)
{
	// TODO(jeysym): Implement
}

Pak Pak::load_from_file(const char* file_path) {
	HANDLE pak_file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (pak_file == INVALID_HANDLE_VALUE) {
		std::cerr << "[Error] Failed to open the pak file." << std::endl;
		std::exit(IO_ERROR);
	}

	LARGE_INTEGER pak_file_size;
	if (GetFileSizeEx(pak_file, &pak_file_size) == 0) {
		std::cerr << "[Error] Can't get pak file size." << std::endl;
		std::exit(IO_ERROR);
	}

	// Allocate big buffer to hold the pak file contents.
	char* pak_buffer = static_cast<char*>(std::malloc(
		static_cast<size_t>(pak_file_size.QuadPart))
		);

	if (pak_buffer == nullptr) {
		std::cerr << "[Error] Failed to allocate buffer for pak file." << std::endl;
		std::exit(MEMORY_ERROR);
	}

	DWORD bytes_read = 0;
	if (ReadFile(pak_file, pak_buffer, pak_file_size.QuadPart, &bytes_read, 0) == FALSE) {
		std::cerr << "[Error] Failed to read the pak file." << std::endl;
		std::exit(IO_ERROR);
	}

	CloseHandle(pak_file);

	Pak pak = Pak::load_from_memory(pak_buffer);
	std::free(pak_buffer);

	return pak;
}