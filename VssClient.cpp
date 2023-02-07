#include "VssClient.h"

#ifdef WIN32
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
#include <locale>
#include <codecvt>
#include <optional>

using namespace std;
using namespace Win32VSSWrapper;

namespace {
constexpr auto VOLUME_BUFFER_MAX_LEN = MAX_PATH;
constexpr auto VOLUME_PATH_MAX_LEN = MAX_PATH + 1;
constexpr auto DEVICE_BUFFER_MAX_LEN = MAX_PATH;
constexpr auto VSS_ID_MAX_LEN = 100;
}

std::optional<std::wstring> VssID2WStr(const VSS_ID& vssID)
{
	WCHAR wVssIDBuf[VSS_ID_MAX_LEN] = { L'\0' };
	int ret = ::StringFromGUID2(vssID, wVssIDBuf, VSS_ID_MAX_LEN);
	if (ret == 0) {
		return std::nullopt;
	}
	return std::make_optional<std::wstring>(wVssIDBuf);
}

std::optional<VSS_ID> VssIDfromWStr(const std::wstring& vssIDWstr)
{
	VSS_ID vssID;
	bool ret = ::CLSIDFromString(vssIDWstr.c_str(), &vssID);
	if (!ret) {
		return std::nullopt;
	}
	return std::make_optional<VSS_ID>(vssID);
}

#define CHECK_HR_RETURN_FALSE(HR, FUNC) \
	do { \
		_com_error err(HR); \
		if (HR != S_OK) { \
			fprintf(stderr, "Failed to call " ## FUNC ## ", %s\n", err.ErrorMessage()); \
			return false; \
		} \
	} \
	while (0)

#define CHECK_HR_RETURN(HR, FUNC, RET) \
	do { \
		_com_error err(HR); \
		if (HR != S_OK) { \
			fprintf(stderr, "Failed to call " ## FUNC ## ", %s\n", err.ErrorMessage()); \
			return RET; \
		} \
	} \
	while (0)


std::wstring Utf8ToUtf16(const std::string& str)
{
	using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
	std::wstring_convert<ConvertTypeX> converterX;
	std::wstring wstr = converterX.from_bytes(str);
	return wstr;
}

std::string Utf16ToUtf8(const std::wstring& wstr)
{
	using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
	std::wstring_convert<ConvertTypeX> converterX;
	return converterX.to_bytes(wstr);
}


VssSnapshotProperty::VssSnapshotProperty(const VSS_SNAPSHOT_PROP& prop)
{
	m_snapshotID = prop.m_SnapshotId;
	m_shapshotSetID = prop.m_SnapshotSetId;
	m_snapshotsCount = prop.m_lSnapshotsCount;
	m_wSnapshotDeviceObject = prop.m_pwszSnapshotDeviceObject;
	m_wOriginVolumeName = prop.m_pwszOriginalVolumeName;
	m_wOriginatingMachine = prop.m_pwszOriginatingMachine;
	m_wServiceMachine = prop.m_pwszServiceMachine;
	m_wExposedName = prop.m_pwszExposedName;
	m_wExposedName = prop.m_pwszExposedPath;
	m_providerID = prop.m_ProviderId;
	m_napshotAttributes = prop.m_lSnapshotAttributes;
	m_createTime = prop.m_tsCreationTimestamp;
	m_status = prop.m_eStatus;
}

bool VssClient::CreateSnapshotW(const std::vector<std::wstring>& wVolumePath)
{
	// TODP
	return false;
}

bool VssClient::CreateSnapshotW(const std::wstring& wVolumePath)
{
	VSS_ID snapshot_id;
	WCHAR volume[MAX_PATH] = { L'\0' };
	wcscpy_s(volume, MAX_PATH, wVolumePath.c_str());
	HRESULT rc = m_components->AddToSnapshotSet(volume, GUID_NULL, &snapshot_id);
    CHECK_HR_RETURN_FALSE(rc, "AddToSnapshotSet");

	/* Generate Snapshot */
	rc = m_components->SetBackupState(true, false, VSS_BT_FULL, false);
	CHECK_HR_RETURN_FALSE(rc, "SetBackupState");

	rc = m_components->PrepareForBackup(&m_async);
	CHECK_HR_RETURN_FALSE(rc, "PrepareForBackup");
	rc = m_async->Wait();
	CHECK_HR_RETURN_FALSE(rc, "m_async->Wait");

	rc = m_components->DoSnapshotSet(&m_async);
	CHECK_HR_RETURN_FALSE(rc, "DoSnapshotSet");
	rc = m_async->Wait();
	CHECK_HR_RETURN_FALSE(rc, "m_async->Wait");

	return BackupCompleteSync();
}

bool VssClient::DeleteSnapshotW(const std::wstring& wShadowID)
{
	// TODO
	return false;
}

std::optional<VssSnapshotProperty> VssClient::GetSnapshotProperty(const VSS_ID& snapshotID)
{
	VSS_SNAPSHOT_PROP snapshotProp;
	HRESULT rc = m_components->GetSnapshotProperties(snapshotID, &snapshotProp);
	CHECK_HR_RETURN(rc, "GetSnapshotProperties", std::nullopt);
	VssSnapshotProperty property(snapshotProp);
	::VssFreeSnapshotProperties(&snapshotProp);
	return std::make_optional<VssSnapshotProperty>(property);
}


VssClient::VssClient()
{
	Init();
	Connect();
}

VssClient::~VssClient()
{
	ReleaseResources();
}

/* initialzie COM */
bool VssClient::Init()
{
	HRESULT rc = ::CoInitialize(nullptr);
    CHECK_HR_RETURN_FALSE(rc, "CoInitialize");
	return true;
}

bool VssClient::Connect()
{
	HRESULT rc = ::CreateVssBackupComponents(&m_components);
	CHECK_HR_RETURN_FALSE(rc, "CreateVssBackupComponents");

	rc = m_components->InitializeForBackup();
	CHECK_HR_RETURN_FALSE(rc, "InitializeForBackup");

	rc = m_components->GatherWriterMetadata(&m_async);
	CHECK_HR_RETURN_FALSE(rc, "GatherWriterMetadata");
	rc = m_async->Wait();
	CHECK_HR_RETURN_FALSE(rc, "m_async->Wait");

	rc = m_components->SetContext(VSS_CTX_BACKUP);
	CHECK_HR_RETURN_FALSE(rc, "SetContext");

	VSS_ID snapshot_set_id;
	rc = m_components->StartSnapshotSet(&snapshot_set_id);
	CHECK_HR_RETURN_FALSE(rc, "StartSnapshotSet");

	return true;
}

bool VssClient::BackupCompleteSync()
{
	if (m_components == nullptr) {
		return false;
	}

	HRESULT rc = m_components->BackupComplete(&m_async);
	CHECK_HR_RETURN_FALSE(rc, "BackupComplete");
	
	rc = m_async->Wait();
	CHECK_HR_RETURN_FALSE(rc, "m_async->Wait");
	
	return true;
}

void VssClient::ReleaseResources()
{
	if (m_components != nullptr) {
		m_components->Release();
		::CoUninitialize();
		m_components = nullptr;
	}
}

#endif