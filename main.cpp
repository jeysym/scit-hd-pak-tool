#include "common.h"
#include "pak.h"
#include <iostream>
#include <filesystem>
#include <string>

static const char* help_message = 
R"(# Usage

scit-hd-pak-tool unpak <pak-path> <dir-path>
    Unpacks .pak file into individual files that will be stored in given directory.

scit-hd-pak-tool repak <dir-path> <pak-path>
    Packs the files in given directory back into .pak file.

# Example

1. Run `scit-hd-pak-tool unpak "c:\program files\santa claus in trouble\data.pak" c:\data_pak`
2. Edit files in directory `c:\data_pak`
3. Run `scit-hd-pak-tool repak "c:\data_pak c:\program files\santa claus in trouble\data.pak"`
)";

int main(int arg_count, char** args)
{
	namespace fs = std::filesystem;

	if (arg_count <= 1) {
		std::cout << help_message;
		return SUCCESS;
	}

	std::string mode{ args[1] };
	if (mode == "unpak" && arg_count == 4) {
		fs::path pak_path{ args[2] };
		fs::path dir_path{ args[3] };

		if (fs::is_regular_file(pak_path) == false) {
			std::cerr << "[Error] File " << pak_path << " does not exist.\n";
			return ARGUMENT_ERROR;
		}

		Pak pak;
		pak.load_from_file(pak_path);
		pak.save_to_dir(dir_path);
		return SUCCESS;
	}

	if (mode == "repak" && arg_count == 4) {
		fs::path dir_path{ args[2] };
		fs::path pak_path{ args[3] };

		if (fs::is_directory(dir_path) == false) {
			std::cerr << "[Error] Directory " << dir_path << " does not exist.\n";
			return ARGUMENT_ERROR;
		}

		Pak pak;
		pak.load_from_dir(dir_path);
		pak.save_to_file(pak_path);
		return SUCCESS;
	}

	std::cerr << "[Error] Wrong arguments specified. Run with no arguments to show help." << std::endl;
	return ARGUMENT_ERROR;
}