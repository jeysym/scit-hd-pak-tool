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
	IO_ERROR
};

int main()
{
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