#include "common.h"
#include "pak.h"
#include "crc.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <windows.h>


int main(int arg_count, char** args)
{
	namespace fs = std::filesystem;

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

	std::string mode{ args[1] };
	if (mode == "pak-to-dir") {
		fs::path pak_path{ args[2] };
		fs::path dir_path = pak_path;
		dir_path.replace_extension();

		Pak pak = Pak::load_from_file(pak_path);
		pak.save_to_dir(dir_path);
		
		return SUCCESS;
	}
	else if (mode == "dir-to-pak") {
		fs::path dir_path{ args[2] };
		fs::path pak_path = dir_path;
		pak_path.replace_extension(".pak");

		Pak pak = Pak::load_from_dir(dir_path);
		pak.save_to_file(pak_path);

		return SUCCESS;
	}
	else {
		std::cerr << "[Error] Wrong arguments specified." << std::endl;
		return ARGUMENT_ERROR;
	}
}