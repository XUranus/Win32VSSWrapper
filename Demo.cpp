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
	std::cout << "vssclient list" << std::endl;
	std::cout << "vssclient create <volumePath>" << std::endl;
	std::cout << "vssclient delete <snapshotID>" << std::endl;
}

void DoListAllCommand()
{
	std::wcout << L"TODO listAllCommand" << std::endl;
}

int DoCreateCommand(const std::wstring& wPath)
{
	std::wcout << L"create snapshot for " << wPath << std::endl;

	WCHAR wVolumePathBuf[MAX_PATH + 1] = { L'\0' };
    if (::GetVolumePathNameW(wPath.c_str(), wVolumePathBuf, MAX_PATH) == FALSE) {
        std::wcout << L"Failed To Get Volume Path For " << wPath << std::endl;
        return -1;
    }
	std::wstring wVolumePath(wVolumePathBuf);
	std::wcout << L"The Volume Path Of " << wPath << L" Is " << wVolumePath << std::endl;
	
	VssClient vssClient;
	std::optional<SnapshotSetResultW> result = vssClient.CreateSnapshotW(wVolumePath);
	if (!result) {
		std::wcout << L"Failed to Create Snapshot" << std::endl;
		return -1;
	} else {
		std::wcout << L"Create Snapshot Success" << std::endl;
	}
	std::wcout << L"Shadow Set ID: " << result->wSnapshotSetID << std::endl;
	int index = 0;
	for (const std::wstring& wSnapshotID: result->wSnapshotIDList) {
		std::wcout << L"snapshot ID[" << ++index << L"]: " << wSnapshotID << std::endl;
	}
	return 0;
}

int DoDeleteSnapshot(const std::wstring& wSnapshotID)
{
	VssClient vssClient;
	bool success = vssClient.DeleteSnapshotW(wSnapshotID);
	if (success) {
		std::wcout << L"Delete Snapshot " << wSnapshotID << L" Success" << std::endl;
		return 0;
	} else {
		std::wcout << L"Delete Snapshot " << wSnapshotID << L" Failed" << std::endl;
		return -1;
	}
	return 0;
}

int wmain(int argc, WCHAR** argv)
{
	::SetConsoleOutputCP(65001); // forcing cmd to use UTF-8 output encoding
	if (argc < 2) {
		PrintHelp();
		std::wcout << L"insufficient paramaters" << std::endl;
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		if (std::wstring(argv[i]) == L"list") {
			return DoListAllCommand();
		}
		else if (std::wstring(argv[i]) == L"create" && i + 1 < argc) {
			return DoCreateCommand(std::wstring(argv[i + 1]));
		} else if (std::wstring(argv[i]) == L"delete" && i + 1 < argc) {
			return DoDeleteSnapshot(std::wstring(argv[i + 1]));
		} else {
			std::wcout << L"Illegal Command" << std::endl;
			PrintHelp();
			return -1;
		}
	}
	return 0;
}


#endif