#include "common.h"
#include "pak.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <windows.h>


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

	std::string mode{ args[1] };
	if (mode == "pak-to-dir") {
		std::string pak_path{ args[2] };
		std::string dir_path{ pak_path.begin(), pak_path.end() - 4 };

		Pak pak = Pak::load_from_file(pak_path.c_str());
		pak.save_to_dir(dir_path.c_str());
		
		return SUCCESS;
	}
	else if (mode == "dir-to-pak") {
		std::string dir_path{ args[2] };
		std::string pak_path = dir_path + ".pak";

		Pak pak = Pak::load_from_dir(dir_path.c_str());
		pak.save_to_file(pak_path.c_str());

		return SUCCESS;
	}
	else {
		std::cerr << "[Error] Wrong arguments specified." << std::endl;
		return ARGUMENT_ERROR;
	}
}