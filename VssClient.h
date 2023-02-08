#ifndef __XURANUS_WIN32_VSS_CLIENT_UTIL_H__
#define __XURANUS_WIN32_VSS_CLIENT_UTIL_H__

#include <string>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objbase.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <comdef.h>

namespace Win32VSSWrapper {

class VssSnapshotProperty {
public:
	VssSnapshotProperty(const VSS_SNAPSHOT_PROP &prop);

	std::wstring SnapshotIDW();
	std::wstring SnapshotSetIDW();
	uint64_t SnapshotsCount();
	std::wstring SnapshotDeviceObjectW();
	std::wstring OriginVolumeNameW();
	std::wstring OriginatingMachineW();
	std::wstring ServiceMachineW();
	std::wstring ExposedNameW();
	std::wstring ExposedPathW();
	std::wstring ProviderIDW();
	uint64_t SnapshotAttributes();
	uint64_t CreateTime();
	VSS_SNAPSHOT_STATE Status();

	/* API for UTF-8 */
	std::string SnapshotID();
	std::string SnapshotSetID();
	std::string ProviderID();
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
	uint64_t m_snapshotAttributes;
	uint64_t m_createTime;
	VSS_SNAPSHOT_STATE m_status;
};


class SnapshotSetResult {
public:
	std::vector<std::wstring> m_wSnapshotIDList;
	std::wstring m_wSnapshotSetID;
public:
	std::vector<std::string> SnapshotIDList() const;
	std::string SnapshotSetID() const;
	std::vector<std::wstring> SnapshotIDListW() const;
	std::wstring SnapshotSetIDW() const;
};

class VssClient {
public:
	VssClient();
	~VssClient();
	std::optional<SnapshotSetResult> CreateSnapshotW(const std::wstring& wVolumePath);
	std::optional<SnapshotSetResult> CreateSnapshot(const std::string& volumePath);
	//bool CreateSnapshotW(const std::vector<std::wstring>& wVolumePath);
	bool DeleteSnapshotW(const std::wstring& wSnapshotID);
	bool DeleteSnapshot(const std::string& snapshotID);
	std::optional<VssSnapshotProperty> GetSnapshotPropertyW(const std::wstring& wSnapshotID);
	std::optional<VssSnapshotProperty> GetSnapshotProperty(const std::string& snapshotID);
private:
	bool Init();
	bool Connect();
	bool WaitAndCheckForAsyncOperation(IVssAsync* pAsync);
	void ReleaseResources();
private:
	bool m_comInitialized = false;
	IVssBackupComponents* m_pVssObject = nullptr;
};

}

#endif
