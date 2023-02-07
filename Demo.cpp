#pragma execution_character_set("utf-8")
#ifdef WIN32

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include "VssClient.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/**
 * reference: https://cs-cjl.com/2018/09_11_backup_file_with_volume_shadow_copy_service
 */
using namespace Win32VSSWrapper;

void PrintHelp()
{
	std::cout << "vssclient, manpulate VSS for windows" << std::endl;
	std::cout << "Usage: " << std::endl;
	std::cout << "vssclient -list" << std::endl;
	std::cout << "vssclient create" << std::endl;
}

int wmain(int argc, WCHAR** argv)
{
	::SetConsoleOutputCP(65001); // forcing cmd to use UTF-8 output encoding
	if (argc < 2) {
		PrintHelp();
		std::wcout << L"insufficient paramaters" << std::endl;
		return 1;
	}

	std::wstring wVolumePath = argv[1];
	WCHAR wVolume[MAX_PATH + 1] = { '\0' };
    if (::GetVolumePathNameW(wVolumePath.c_str(), wVolume, MAX_PATH) == FALSE) {
        fprintf(stderr, "Failed to GetVolumePathNameW\n");
        return -1;
    }
	std::wcout << "volumePath: " << std::wstring(wVolume) << std::endl;
	

	VssClient VssClient;
	VssClient.CreateSnapshotW(wVolume);
}


#endif