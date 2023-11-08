#include "pak.h"
#include "crc.h"
#include <windows.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
Crc32 g_crc32;


static bool path_alphabetical_compare(const fs::path& a, const fs::path& b) {
	const auto& a_str = a.native();
	const auto& b_str = b.native();

	using path_char_t = fs::path::string_type::value_type;
	return std::lexicographical_compare(
		a_str.begin(), a_str.end(),
		b_str.begin(), b_str.end(),
		[](path_char_t a, path_char_t b) -> bool { return std::toupper(a) < std::toupper(b); }
	);
}

static bool dir_entry_alphabetical_compare(const fs::directory_entry& a, const fs::directory_entry& b) {
	return path_alphabetical_compare(a.path(), b.path());
}


// Data structures serialization
// =============================

void File::load_from_memory(const char*& data)
{
	// Parse the header
	name_length = get_uint32(data);
	file_size = get_uint64(data);
	pak_offset = get_uint64(data);
	crc32 = get_uint32(data);
	zero = get_uint32(data);
	assert(zero == 0);
	name = get_string(data);
	assert((name.size() + 1) == name_length);
}

void File::save_to_memory(char*& data) const
{
	set_uint32(data, name_length);
	set_uint64(data, file_size);
	set_uint64(data, pak_offset);
	set_uint32(data, crc32);
	set_uint32(data, zero);
	set_string(data, name);
}

size_t File::get_header_size() const
{
	return sizeof(name_length) + sizeof(file_size) + sizeof(pak_offset) + sizeof(crc32) + sizeof(zero) + name_length;
}

void Dir::load_from_memory(const char*& data)
{
	// Parse the header
	dir_index = get_uint32(data);
	name_length = get_uint32(data);
	num_of_files = get_uint32(data);
	zero = get_uint32(data);
	assert(zero == 0);
	name = get_string(data);
	assert((name.size() + 1) == name_length);

	// Parse the files
	files.resize(num_of_files);
	for (uint file_idx = 0; file_idx < num_of_files; ++file_idx)
	{
		files[file_idx].load_from_memory(data);
	}
}

void Dir::save_to_memory(char*& data) const
{
	set_uint32(data, dir_index);
	set_uint32(data, name_length);
	set_uint32(data, num_of_files);
	set_uint32(data, zero);
	set_string(data, name);

	// Save the files
	for (uint file_idx = 0; file_idx < num_of_files; ++file_idx)
	{
		files[file_idx].save_to_memory(data);
	}
}

size_t Dir::get_header_size() const
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
	name = "\\" + path2 + "\\";									// "\\dir_a\\dir_b\\"
}

void Pak::load_from_memory(const char* data)
{
	const char* data_base = data;

	// Parse the header
	tag = get_uint64(data);
	base_offset = get_uint64(data);
	num_dirs = get_uint32(data);
	zero = get_uint32(data);
	assert(zero == 0);

	// Parse the directories
	directories.resize(num_dirs);
	for (uint dir_idx = 0; dir_idx < num_dirs; ++dir_idx) {
		Dir& current_dir = directories[dir_idx];

		current_dir.load_from_memory(data);

		// Copy the actual file contents.
		for (uint file_idx = 0; file_idx < current_dir.num_of_files; ++file_idx) {
			File& current_file = current_dir.files[file_idx];

			current_file.data = static_cast<char*>(std::malloc(current_file.file_size));
			std::memcpy(current_file.data, data_base + current_file.pak_offset, current_file.file_size);
		}
	}
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
	for (uint dir_idx = 0; dir_idx < num_dirs; ++dir_idx) {
		directories[dir_idx].save_to_memory(data);

		// Copy the actual file contents.
		const Dir& current_dir = directories[dir_idx];
		for (uint file_idx = 0; file_idx < current_dir.num_of_files; ++file_idx) {
			const File& current_file = current_dir.files[file_idx];

			std::memcpy(data_base + current_file.pak_offset, current_file.data, current_file.file_size);
		}
	}
}

size_t Pak::get_total_headers_size() const
{
	size_t total_size = 0;

	total_size += get_header_size();
	for (const auto& dir : directories) {
		total_size += dir.get_header_size();
		for (const auto& file : dir.files) {
			total_size += file.get_header_size();
		}
	}

	return total_size;
}

size_t Pak::get_total_files_size() const
{
	size_t total_size = 0;

	for (const Dir& dir : directories) {
		for (const File& file : dir.files) {
			total_size += file.file_size;
		}
	}

	return total_size;
}

size_t Pak::get_total_pak_size() const
{
	return get_total_headers_size() + get_total_files_size();
}

size_t Pak::get_header_size() const
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
	result.num_dirs = static_cast<uint32_t>(dir_to_files.size());
	result.zero = 0;

	result.directories.resize(result.num_dirs);

	auto map_it = dir_to_files.begin();
	for (uint dir_idx = 0; dir_idx < result.num_dirs; ++dir_idx) {
		Dir& dir = result.directories[dir_idx];
		fs::path curr_dir_path = map_it->first;
		auto& curr_dir_files = map_it->second;

		std::sort(curr_dir_files.begin(), curr_dir_files.end(), dir_entry_alphabetical_compare);

		dir.dir_index = dir_idx + 1;
		dir.set_dir_path(curr_dir_path);
		dir.name_length = static_cast<uint32_t>(dir.name.size() + 1);
		dir.num_of_files = static_cast<uint32_t>(curr_dir_files.size());
		dir.zero = 0;

		dir.files.resize(dir.num_of_files);

		for (uint file_idx = 0; file_idx < dir.num_of_files; ++file_idx) {
			File& file = dir.files[file_idx];
			const fs::directory_entry& file_dir_entry = curr_dir_files[file_idx];

			file.name = file_dir_entry.path().filename().string();
			file.name_length = static_cast<uint32_t>(file.name.size() + 1);
			file.file_size = static_cast<uint32_t>(file_dir_entry.file_size());
			file.zero = 0;
		}

		map_it++;
	}

	result.base_offset = result.get_total_headers_size();
	size_t file_offset = result.base_offset;

	for (auto& dir : result.directories) {
		for (auto& file : dir.files) {
			file.pak_offset = file_offset;
			file_offset += file.file_size;

			file.data = static_cast<char*>(std::malloc(file.file_size));

			// read the file here into buffer.
			fs::path file_path = dir_path / dir.get_dir_path() / fs::path(file.name);
			HANDLE file_handle = CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			DWORD bytes_read = 0;
			ReadFile(file_handle, file.data, static_cast<DWORD>(file.file_size), &bytes_read, 0);
			file.crc32 = g_crc32.calculate(file.data, file.file_size);
		}
	}

	return result;
}

void Pak::save_to_file(std::filesystem::path file_path) const
{
	size_t total_size = get_total_pak_size();

	char* buffer = static_cast<char*>(std::malloc(total_size));
	save_to_memory(buffer);

	HANDLE file = CreateFile(file_path.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD bytes_written = 0;
	WriteFile(file, buffer, static_cast<DWORD>(total_size), &bytes_written, 0);
}

void Pak::save_to_dir(std::filesystem::path dir_path) const
{
	for (uint dir_idx = 0; dir_idx < num_dirs; ++dir_idx) {
		const Dir& current_dir = directories[dir_idx];

		for (uint file_idx = 0; file_idx < current_dir.num_of_files; ++file_idx) {
			const File& current_file = current_dir.files[file_idx];

			fs::path target_dir = dir_path / current_dir.get_dir_path();
			fs::path target_file = target_dir / fs::path(current_file.name);

			std::filesystem::create_directories(target_dir);

			HANDLE file = CreateFile(target_file.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			DWORD bytes_written = 0;
			WriteFile(file, current_file.data, static_cast<DWORD>(current_file.file_size), &bytes_written, 0);
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

	// TODO(jeysym): This will probably fail if the file is bigger that 4GiB.
	DWORD bytes_read = 0;
	if (ReadFile(pak_file, pak_buffer, pak_file_size.LowPart, &bytes_read, 0) == FALSE) {
		std::cerr << "[Error] Failed to read the pak file." << std::endl;
		std::exit(IO_ERROR);
	}

	CloseHandle(pak_file);

	Pak pak;
	pak.load_from_memory(pak_buffer);
	std::free(pak_buffer);

	return pak;
}