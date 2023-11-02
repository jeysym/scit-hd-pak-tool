#include "pak.h"
#include <windows.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Data structures serialization
// =============================

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

void File::save_to_memory(char*& data) const
{
	set_uint32(data, name_length);
	set_uint64(data, size);
	set_uint64(data, pak_offset);
	set_uint32(data, crc32);
	set_uint32(data, zero);
	set_string(data, name);
}

size_t File::size_in_pak() const
{
	return sizeof(name_length) + sizeof(size) + sizeof(pak_offset) + sizeof(crc32) + sizeof(zero) + name_length;
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

void Dir::save_to_memory(char*& data) const
{
	set_uint32(data, dir_index);
	set_uint32(data, name_length);
	set_uint32(data, num_of_files);
	set_uint32(data, zero);
	set_string(data, name);

	// Save the files
	for (int file_idx = 0; file_idx < num_of_files; ++file_idx)
	{
		files[file_idx].save_to_memory(data);
	}
}

size_t Dir::size_in_pak() const
{
	return sizeof(dir_index) + sizeof(name_length) + sizeof(num_of_files) + sizeof(zero) + name_length;
}

std::filesystem::path Dir::get_dir_path() const
{
	std::string rel_dir_path = name.substr(1, name.size() - 2);	// Drop the first and last '\\'.
	return fs::path{ rel_dir_path };
}

void Dir::set_dir_path(std::filesystem::path dir_path)
{
	std::string path1 = dir_path.string();						// "pak_folder\\dir_a\\dir_b"
	std::string path2 = path1.substr(path1.find('\\') + 1);		// "dir_a\\dir_b"
	std::string path3 = replace_all(path2, "\\", "\\\\");		// "dir_a\\\\dir_b"
	name = "\\" + path3 + "\\";									// "\\dir_a\\\\dir_b"
}

Pak Pak::load_from_memory(const char* data)
{
	const char* data_base = data;
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
			std::memcpy(current_file.data, data_base + current_file.pak_offset, current_file.size);
		}
	}

	return result;
}

void Pak::save_to_memory(char* data) const
{
	char* data_base = data;

	// Save the header
	set_uint64(data, tag);
	set_uint64(data, base_offset);
	set_uint32(data, num_dirs);
	set_uint32(data, zero);

	// Save the directories
	for (int dir_idx = 0; dir_idx < num_dirs; ++dir_idx) {
		directories[dir_idx].save_to_memory(data);

		// Copy the actual file contents.
		const Dir& current_dir = directories[dir_idx];
		for (int file_idx = 0; file_idx < current_dir.num_of_files; ++file_idx) {
			const File& current_file = current_dir.files[file_idx];

			current_file.save_to_memory(data);
			std::memcpy(data_base + current_file.pak_offset, current_file.data, current_file.size);
		}
	}
}

size_t Pak::size_in_pak() const
{
	return sizeof(tag) + sizeof(base_offset) + sizeof(num_dirs) + sizeof(zero);
}


// Loading/saving of data structures from/to files and directories.
// ================================================================

Pak Pak::load_from_dir(std::filesystem::path dir_path)
{
	Pak result;

	std::map<fs::path, std::vector<fs::directory_entry>> dir_to_files;

	for (const auto& dir_entry : fs::recursive_directory_iterator(dir_path)) {
		if (dir_entry.is_regular_file()) {
			fs::path parent_path = dir_entry.path().parent_path();
			dir_to_files[parent_path].push_back(dir_entry);
		}
	}

	result.tag = Pak::TAG_MAGIC;
	result.num_dirs = dir_to_files.size();
	result.zero = 0;

	result.directories.resize(result.num_dirs);

	auto map_it = dir_to_files.begin();
	for (int dir_idx = 0; dir_idx < result.num_dirs; ++dir_idx) {
		Dir& dir = result.directories[dir_idx];
		fs::path curr_dir_path = map_it->first;
		auto& curr_dir_files = map_it->second;

		std::sort(curr_dir_files.begin(), curr_dir_files.end());

		dir.dir_index = dir_idx + 1;
		dir.set_dir_path(curr_dir_path);
		dir.name_length = dir.name.size() + 1;
		dir.num_of_files = curr_dir_files.size();
		dir.zero = 0;

		dir.files.resize(dir.num_of_files);

		for (int file_idx = 0; file_idx < dir.num_of_files; ++file_idx) {
			File& file = dir.files[file_idx];
			const fs::directory_entry& file_dir_entry = curr_dir_files[file_idx];

			file.name = file_dir_entry.path().filename().string();
			file.name_length = file.name.size() + 1;
			file.size = file_dir_entry.file_size();
			file.zero = 0;
		}

		map_it++;
	}

	size_t base_offset = 0;
	base_offset += result.size_in_pak();
	for (const auto& dir : result.directories) {
		base_offset += dir.size_in_pak();
		for (const auto& file : dir.files) {
			base_offset += file.size_in_pak();
		}
	}

	result.base_offset = base_offset;
	size_t file_offset = base_offset;

	for (auto& dir : result.directories) {
		for (auto& file : dir.files) {
			file.pak_offset = file_offset;
			file_offset += file.size;

			file.data = static_cast<char*>(std::malloc(file.size));

			// read the file here into buffer.
			fs::path file_path = dir_path / dir.get_dir_path() / fs::path(file.name);
			HANDLE file_handle = CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			DWORD bytes_read = 0;
			ReadFile(file_handle, file.data, file.size, &bytes_read, 0);
		}
	}

	return result;
}

void Pak::save_to_file(std::filesystem::path file_path) const
{
	size_t total_size = 0;
	total_size += size_in_pak();	// for pak header

	for (const Dir& dir : directories) {
		total_size += dir.size_in_pak();

		for (const File& file : dir.files) {
			total_size += file.size_in_pak();
			total_size += file.size;
		}
	}

	char* buffer = static_cast<char*>(std::malloc(total_size));
	save_to_memory(buffer);

	HANDLE file = CreateFile(file_path.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD bytes_written = 0;
	WriteFile(file, buffer, total_size, &bytes_written, 0);
}

void Pak::save_to_dir(std::filesystem::path dir_path) const
{
	for (int dir_idx = 0; dir_idx < num_dirs; ++dir_idx) {
		const Dir& current_dir = directories[dir_idx];

		for (int file_idx = 0; file_idx < current_dir.num_of_files; ++file_idx) {
			const File& current_file = current_dir.files[file_idx];

			fs::path target_dir = dir_path / current_dir.get_dir_path();
			fs::path target_file = target_dir / fs::path(current_file.name);

			std::filesystem::create_directories(target_dir);

			HANDLE file = CreateFile(target_file.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			DWORD bytes_written = 0;
			WriteFile(file, current_file.data, current_file.size, &bytes_written, 0);
		}
	}
}

Pak Pak::load_from_file(std::filesystem::path file_path) {
	HANDLE pak_file = CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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