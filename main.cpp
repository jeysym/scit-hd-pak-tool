#include "common.h"
#include "pak.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <windows.h>


enum Error {
	SUCCESS = 0,
	MEMORY_ERROR,
	IO_ERROR,
	ARGUMENT_ERROR
};

int main(int arg_count, char** args)
{
	if (arg_count != 3) {
		std::cout
			<< "Usage:\n"
			<< "\n"
			<< "scit-hd-pak-tool pak-to-dir example.pak\n"
			<< "    Unpacks file 'exaple.pak' into individual files that will be stored in directory 'example'.\n"
			<< "\n"
			<< "scit-hd-pak-tool dir-to-pak example\n"
			<< "    Packs the files in directory 'example' into file 'example.pak'.\n";
		return SUCCESS;
	}

	const char* mode = args[1];
	if (std::strcmp(mode, "pak-to-dir") != 0) {
		const char* pak_path = args[2];

		// TODO(jeysym): Load pak and save it to dir.
	}
	else if (std::strcmp(mode, "dir-to-pak") != 0) {
		const char* dir_path = args[2];

		// TODO(jeysym): Load dir and save it to pak.
	}
	else {
		std::cerr << "[Error] Wrong arguments specified." << std::endl;
		return ARGUMENT_ERROR;
	}


	HANDLE pak_file = CreateFile(L"data_win32.pak", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (pak_file == INVALID_HANDLE_VALUE) {
		std::cerr << "[Error] Failed to open the pak file." << std::endl;
		return IO_ERROR;
	}

	LARGE_INTEGER pak_file_size;
	if (GetFileSizeEx(pak_file, &pak_file_size) == 0) {
		std::cerr << "[Error] Can't get pak file size." << std::endl;
		return IO_ERROR;
	}

	// Allocate big buffer to hold the pak file contents.
	char* pak_buffer = static_cast<char*>(std::malloc(pak_file_size.QuadPart));

	if (pak_buffer == nullptr) {
		std::cerr << "[Error] Failed to allocate buffer for pak file." << std::endl;
		return MEMORY_ERROR;
	}

	DWORD bytes_read = 0;
	if (ReadFile(pak_file, pak_buffer, pak_file_size.QuadPart, &bytes_read, 0) == FALSE) {
		std::cerr << "[Error] Failed to read the pak file." << std::endl;
		return IO_ERROR;
	}

	CloseHandle(pak_file);

	Pak pak;
	pak.load(pak_buffer);

	std::free(pak_buffer);

	return SUCCESS;
}