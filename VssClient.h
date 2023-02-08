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

	uint64_t SnapshotAttributes() const;
	uint64_t CreateTime() const;
	VSS_SNAPSHOT_STATE Status() const;
	uint64_t SnapshotsCount() const;
	bool ClientAccessible() const;

	/* API for UTF-16 */
	std::wstring SnapshotIDW() const;
	std::wstring SnapshotSetIDW() const;
	std::wstring SnapshotDeviceObjectW() const;
	std::wstring OriginVolumeNameW() const;
	std::wstring OriginatingMachineW() const;
	std::wstring ServiceMachineW() const;
	std::wstring ExposedNameW() const;
	std::wstring ExposedPathW() const;
	std::wstring ProviderIDW() const;

	/* API for UTF-8 */
	std::string SnapshotID() const;
	std::string SnapshotSetID() const;
	std::string ProviderID() const;
	std::string SnapshotDeviceObject() const;
	std::string OriginVolumeName() const;
	std::string OriginatingMachine() const;
	std::string ServiceMachine() const;
	std::string ExposedName() const;
	std::string ExposedPath() const;
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
	bool ExposeSnapshotLocallyW(const std::wstring& snapshotID, const std::wstring& wPath);
	bool ExposeSnapshotLocally(const std::string& snapshotID, const std::string& path);
private:
	bool InitializeCom(); /* register this thread to Win32 COM */
	bool Connect(); /* connect to Volume Shadows Service */
	bool WaitAndCheckForAsyncOperation(IVssAsync* pAsync);
	void ReleaseResources();
private:
	bool m_comInitialized = false;
	IVssBackupComponents* m_pVssObject = nullptr;
};

/**
 * An Util Class, Used to automatically release a CoTaskMemAlloc allocated pointer
 * when the instance of this class goes out of scope（RAII)
 * (even if an exception is thrown)
 **/
class CAutoComPointer
{
public:
    CAutoComPointer(LPVOID ptr): m_ptr(ptr) {};
    ~CAutoComPointer() { ::CoTaskMemFree(m_ptr); }
private:
    LPVOID m_ptr;
};

}

#endif
