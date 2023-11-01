#include "pak.h"

void File::load(const char*& data)
{
	// Parse the header
	name_length = get_uint32(data);
	size = get_uint64(data);
	pak_offset = get_uint64(data);
	crc32 = get_uint32(data);
	zero = get_uint32(data);
	assert(zero == 0);
	name = get_string(data);
	assert((name.size() + 1) == name_length);
}

void Dir::load(const char*& data)
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
	for (int file_idx = 0; file_idx < num_of_files; ++file_idx)
	{
		File file;
		file.load(data);
		files.push_back(file);
	}
}

void Pak::load(const char* data)
{
	// Parse the header
	tag = get_uint64(data);
	base_offset = get_uint64(data);
	num_dirs = get_uint32(data);
	zero = get_uint32(data);
	assert(zero == 0);

	// Parse the directories
	for (int dir_idx = 0; dir_idx < num_dirs; ++dir_idx) {
		Dir dir;
		dir.load(data);
		directories.push_back(dir);
	}
}