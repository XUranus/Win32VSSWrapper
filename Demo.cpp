#pragma execution_character_set("utf-8")
#ifdef WIN32

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <set>
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
	std::cout << "vssclient query <snapshotID>" << std::endl;
	std::cout << "vssclient create <volumePath1> <volumePath2> ..." << std::endl;
	std::cout << "vssclient delete <snapshotID>" << std::endl;
	std::cout << "vssclient delete all" << std::endl;
	std::cout << "vssclient delset <snapshotSetID>" << std::endl;
	std::cout << "vssclient mount <snapshotID> <path>" << std::endl;
	std::cout << "vssclient umount <snapshotID>" << std::endl;
	std::cout << "vssclient mkwritable <snapshotID>" << std::endl;
}

void PrintVssSnapshotPropertyAttributes(const VssSnapshotProperty& property)
{
	std::wcout << L"SnapshotAttributes: ";
	if (property.IsClientAccessible()) {
		std::wcout << L"ClientAccessible ";
	}
	if (property.IsExposedLocally()) {
		std::wcout << L"ExposedLocally ";
	}
	if (property.IsExposedRemotely()) {
		std::wcout << L"ExposedRemotely ";
	}
	if (property.IsTransportable()) {
		std::wcout << L"Transportable ";
	}
	if (property.IsNoAutoRelease()) {
		std::wcout << L"NoAutoRelease ";
	} else {
		std::wcout << L"AutoRelease ";
	}
	if (property.IsPersistent()) {
		std::wcout << L"Persistent "; 
	}
	if (property.IsHardwareAssisted()) {
		std::wcout << L"HardwareAssisted ";
	}
	if (property.IsNoWriters()) {
		std::wcout << L"NoWriters ";
	}
	if (property.IsImported()) {
		std::wcout << L"Imported "; 
	}
	if (property.IsPlex()) {
		std::wcout << L"Plex ";
	}
	if (property.IsDifferential()) {
		std::wcout << L"Differential ";
	}
	std::wcout << std::endl;
}

void PrintVssSnapshotProperty(const VssSnapshotProperty& property)
{
	std::wcout << L"SnapshotID: " << property.SnapshotIDW() << std::endl;
	std::wcout << L"SnapshotSetID: " << property.SnapshotSetIDW() << std::endl;
	std::wcout << L"SnapshotsCount: " << property.SnapshotsCount() << std::endl;
	std::wcout << L"SnapshotDeviceObject: " << property.SnapshotDeviceObjectW() << std::endl;
	std::wcout << L"OriginVolumeName: " << property.OriginVolumeNameW() << std::endl;
	std::wcout << L"OriginatingMachine: " << property.OriginatingMachineW() << std::endl;
	std::wcout << L"ServiceMachine: " << property.ServiceMachineW() << std::endl;
	std::wcout << L"ExposedName: " << property.ExposedNameW() << std::endl;
	std::wcout << L"ExposedPath: " << property.ExposedPathW() << std::endl;
	std::wcout << L"ProviderID: " << property.ProviderIDW() << std::endl;
	std::wcout << L"CreateTime: " << property.CreateTime() << std::endl;
	std::wcout << L"Status: " << property.Status() << std::endl;
	PrintVssSnapshotPropertyAttributes(property);
}

int DoCommandListAll()
{
	VssClient vssClient;
	std::vector<VssSnapshotProperty> properties = vssClient.QueryAllSnapshots();
	if (properties.empty()) {
		std::wcout << L"No Snapshots Found!" << std::endl;
		return 0;
	} else {
		std::wcout << properties.size() << " Snapshots Found: \n" << std::endl;
	}
	for (const VssSnapshotProperty& property: properties) {
		PrintVssSnapshotProperty(property);
		std::wcout << std::endl;
	}
	return 0;
}

int DoCommandQuerySnapshot(const std::wstring& wSnapshotID)
{
	std::wcout << L"Query Snapshot ID " << wSnapshotID << std::endl;
	VssClient vssClient;
	std::optional<VssSnapshotProperty> property = vssClient.GetSnapshotPropertyW(wSnapshotID);
	if (!property) {
		std::wcout << L"Failed To Query Property" << std::endl;
		return -1;
	}
	PrintVssSnapshotProperty(property.value());
	return 0;
}

int DoCommandCreate(const std::vector<std::wstring>& wPathList)
{
	std::set<std::wstring> wVolumePathSet;
	for (const std::wstring& wPath: wPathList) {
		WCHAR wVolumePathBuf[MAX_PATH + 1] = { L'\0' };
		if (::GetVolumePathNameW(wPath.c_str(), wVolumePathBuf, MAX_PATH) == FALSE) {
			std::wcout << L"Failed To Get Volume Path For " << wPath << std::endl;
			return -1;
		}
		std::wstring wVolumePath(wVolumePathBuf);
		std::wcout << L"The Volume Path Of " << wPath << L" Is " << wVolumePath << std::endl;
		wVolumePathSet.insert(wVolumePath);
	}

	std::wcout << L"===== Begin To Create Snapshots =====" << std::endl;
	std::vector<std::wstring> wVolumePathList(wVolumePathSet.begin(), wVolumePathSet.end());
	for (const std::wstring& wVolumePath: wVolumePathList) {
		std::wcout << L"create snapshot for " << wVolumePath << std::endl;
	}
	if (wVolumePathList.empty()) {
		std::wcout << L"Volume Path List Is Empty" << std::endl;
		return -1;
	}
	
	VssClient vssClient;
	std::optional<SnapshotSetResult> result = vssClient.CreateSnapshotsW(wVolumePathList);
	if (!result) {
		std::wcout << L"Failed to Create Snapshots" << std::endl;
		return -1;
	} else {
		std::wcout << L"Create Snapshots Success" << std::endl;
	}
	std::wcout << L"Shadow Set ID: " << result->SnapshotSetIDW() << std::endl;
	int index = 0;
	for (const std::wstring& wSnapshotID: result->SnapshotIDListW()) {
		std::wcout << L"snapshot ID[" << ++index << L"]: " << wSnapshotID << std::endl;
	}
	return 0;
}

int DoCommandDeleteSnapshot(const std::wstring& wSnapshotID)
{
	VssClient vssClient;
	if (wSnapshotID == L"all") {
		if (vssClient.DeleteAllSnapshots()) {
			std::wcout << L"Delete All Snapshot Success" << std::endl;
			return 0;
		} else {
			std::wcout << L"Delete All Snapshot Failed" << std::endl;
			return -1;
		}
	}
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

int DoCommandDeleteSnapshotSet(const std::wstring& wSnapshotSetID)
{
	VssClient vssClient;
	bool success = vssClient.DeleteSnapshotSetW(wSnapshotSetID);
	if (success) {
		std::wcout << L"Delete Snapshot Set " << wSnapshotSetID << L" Success" << std::endl;
		return 0;
	} else {
		std::wcout << L"Delete Snapshot Set " << wSnapshotSetID << L" Failed" << std::endl;
		return -1;
	}
	return 0;
}

int DoCommandMountSnapshot(const std::wstring& wSnapshotID, const std::wstring& wPath)
{
	VssClient vssClient;
	std::optional<std::wstring> exposeNameW = vssClient.ExposeSnapshotLocallyW(wSnapshotID, wPath);
	if (exposeNameW) {
		std::wcout << L"Expose Name: " << exposeNameW.value() << std::endl;
		return 0;
	} else {
		std::wcout << L"Mount Failed" << std::endl;
		return -1;
	}
	return 0;
}

int DoCommandUmountSnapshot(const std::wstring& wSnapshotID)
{
	VssClient vssClient;
	bool success = vssClient.UnExposeSnapshotW(wSnapshotID);
	if (success) {
		std::wcout << L"Umount Success" << std::endl;
		return 0;
	} else {
		std::wcout << L"Umount Failed" << std::endl;
		return -1;
	}
	return 0;
}

int DoCommandSnapshotWritable(const std::wstring& wSnapshotID)
{
	VssClient vssClient;
	bool success = vssClient.MakeSnapshotWritableW(wSnapshotID);
	if (success) {
		std::wcout << L"Make writable Success" << std::endl;
		return 0;
	} else {
		std::wcout << L"Make writable Failed" << std::endl;
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
		const std::wstring wOption = std::wstring(argv[i]);
		if (wOption == L"list") {
			return DoCommandListAll();
		} else if (wOption == L"query" && i + 1 < argc) {
			return DoCommandQuerySnapshot(std::wstring(argv[i + 1]));
		} else if (wOption == L"create" && i + 1 < argc) {
			std::vector<std::wstring> wPathList;
			for (int j = i + 1; j < argc; j++) {
				wPathList.emplace_back(std::wstring(argv[j]));
			}
			return DoCommandCreate(wPathList);
		} else if (wOption == L"delete" && i + 1 < argc) {
			return DoCommandDeleteSnapshot(std::wstring(argv[i + 1]));
		} else if (wOption == L"delset" && i + 1 < argc) {
			return DoCommandDeleteSnapshotSet(std::wstring(argv[i + 1]));
		} else if (wOption == L"mount" && i + 2 < argc) {
			return DoCommandMountSnapshot(std::wstring(argv[i + 1]), std::wstring(argv[i + 2]));
		} else if (wOption == L"mount" && i + 2 < argc) {
			return DoCommandMountSnapshot(std::wstring(argv[i + 1]), std::wstring(argv[i + 2]));
		} else if (wOption == L"umount" && i + 1 < argc) {
			return DoCommandUmountSnapshot(std::wstring(argv[i + 1]));
		} else if (wOption == L"mkwritable" && i + 1 < argc) {
			return DoCommandSnapshotWritable(std::wstring(argv[i + 1]));
		}
		else {
			std::wcout << L"Illegal Command" << std::endl;
			PrintHelp();
			return -1;
		}
	}
	return 0;
}


#endif