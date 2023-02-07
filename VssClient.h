#ifndef __XURANUS_WIN32_VSS_CLIENT_UTIL_H__
#define __XURANUS_WIN32_VSS_CLIENT_UTIL_H__

#include <string>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objbase.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <comdef.h>

namespace Win32VSSWrapper {

std::optional<std::wstring> VssID2WStr(const VSS_ID& vssID);
std::optional<VSS_ID> VssIDfromWStr(const std::wstring& vssID);

class VssSnapshotProperty {
public:
	VssSnapshotProperty(const VSS_SNAPSHOT_PROP &prop);

	VSS_ID SnapshotID();
	VSS_ID SnapshotSetID();
	uint64_t SnapshotsCount();
	std::wstring SnapshotDeviceObjectW();
	std::wstring OriginVolumeNameW();
	std::wstring OriginatingMachineW();
	std::wstring ServiceMachineW();
	std::wstring ExposedNameW();
	std::wstring ExposedPathW();
	VSS_ID ProviderID();
	uint64_t SnapshotAttributes();
	uint64_t CreateTime();
	VSS_SNAPSHOT_STATE Status();

	std::string SnapshotDeviceObject();
	std::string OriginVolumeName();
	std::string OriginatingMachine();
	std::string ServiceMachine();
	std::string ExposedName();
	std::string ExposedPath();
private:
	VSS_ID m_snapshotID;
	VSS_ID m_shapshotSetID;
	uint64_t m_snapshotsCount;
	std::wstring m_wSnapshotDeviceObject;
	std::wstring m_wOriginVolumeName;
	std::wstring m_wOriginatingMachine;
	std::wstring m_wServiceMachine;
	std::wstring m_wExposedName;
	std::wstring m_wExposedPath;
	VSS_ID m_providerID;
	uint64_t m_napshotAttributes;
	uint64_t m_createTime;
	VSS_SNAPSHOT_STATE m_status;
};

class VssClient {
public:
	VssClient();
	~VssClient();
	bool CreateSnapshotW(const std::wstring& wVolumePath);
	bool CreateSnapshotW(const std::vector<std::wstring>& wVolumePath);
	bool DeleteSnapshotW(const std::wstring& wShadowID);
	std::optional<VssSnapshotProperty> GetSnapshotProperty(const VSS_ID& snapshotID);
private:
	bool Init();
	bool Connect();
	bool BackupCompleteSync();
	void ReleaseResources();
private:
	IVssBackupComponents* m_components = nullptr;
	IVssAsync* m_async = nullptr;
};

}

#endif
