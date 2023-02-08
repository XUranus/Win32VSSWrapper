#include "VssClient.h"

#ifdef WIN32
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
#include <locale>
#include <codecvt>
#include <optional>
#include <atlbase.h>

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
	LPOLESTR wVssIDBuf = nullptr;
	HRESULT hr = ::StringFromIID(vssID, &wVssIDBuf);
	if (FAILED(hr)) {
		return std::nullopt;
	}
	std::wstring wVssIDStr(wVssIDBuf);
	::CoTaskMemFree(wVssIDBuf);
	wVssIDBuf = nullptr;
	return std::make_optional<std::wstring>(wVssIDStr);
}

std::optional<VSS_ID> VssIDfromWStr(const std::wstring& vssIDWstr)
{
	VSS_ID vssID;
	HRESULT hr = ::IIDFromString(vssIDWstr.c_str(), &vssID);
	if (FAILED(hr)) {
		return std::nullopt;
	}
	return std::make_optional<VSS_ID>(vssID);
}

#define CHECK_HR_RETURN_FALSE(HR, FUNC) \
	do { \
		_com_error err(HR); \
		if (FAILED(HR)) { \
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

#define CHECK_BOOL_RETURN(BOOLVALUE, FUNC, RET) \
	do { \
		if ((!BOOLVALUE)) { \
			fprintf(stderr, "Failed to call " ## FUNC ## "\n"); \
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

std::wstring VssSnapshotProperty::SnapshotDeviceObjectW() { return m_wSnapshotDeviceObject; }
std::wstring VssSnapshotProperty::OriginVolumeNameW() { return m_wOriginVolumeName; }
std::wstring VssSnapshotProperty::OriginatingMachineW() { return m_wOriginatingMachine; }
std::wstring VssSnapshotProperty::ServiceMachineW() { return m_wServiceMachine; }
std::wstring VssSnapshotProperty::ExposedNameW() { return m_wExposedName; }
std::wstring VssSnapshotProperty::ExposedPathW() { return m_wExposedPath; }

// std::optional<std::wstring> VssClient::CreateSnapshotW(const std::vector<std::wstring>& wVolumePath)
// {
// 	// TODP
// 	return false;
// }

std::vector<std::string> SnapshotSetResult::SnapshotIDList() const
{
	std::vector<std::string> snapshotIDList;
	for (const std::wstring& wSnapshotID: m_wSnapshotIDList) {
		snapshotIDList.emplace_back(Utf16ToUtf8(wSnapshotID));
	}
	return snapshotIDList;
}

std::string SnapshotSetResult::SnapshotSetID() const
{
	return Utf16ToUtf8(m_wSnapshotSetID);
}

std::vector<std::wstring> SnapshotSetResult::SnapshotIDListW() const
{
	return m_wSnapshotIDList;
}

std::wstring SnapshotSetResult::SnapshotSetIDW() const
{
	return m_wSnapshotSetID;
}

std::optional<SnapshotSetResult> VssClient::CreateSnapshotW(const std::wstring& wVolumePath)
{
	// rc = m_pVssObject->GatherWriterMetadata(&m_async);
	// CHECK_HR_RETURN_FALSE(rc, "GatherWriterMetadata");
	// rc = m_async->Wait();
	// CHECK_HR_RETURN_FALSE(rc, "m_async->Wait");

	VSS_ID snapshotSetID;
	HRESULT rc = m_pVssObject->StartSnapshotSet(&snapshotSetID);
	CHECK_HR_RETURN(rc, "StartSnapshotSet", std::nullopt);

	VSS_ID snapshotID;
	WCHAR volume[MAX_PATH] = { L'\0' };
	wcscpy_s(volume, MAX_PATH, wVolumePath.c_str());
	rc = m_pVssObject->AddToSnapshotSet(volume, GUID_NULL, &snapshotID);
    CHECK_HR_RETURN(rc, "AddToSnapshotSet", std::nullopt);

	/* Generate Snapshot */ // TODO: move to init
	rc = m_pVssObject->SetBackupState(true, false, VSS_BT_FULL, false);
	CHECK_HR_RETURN(rc, "SetBackupState", std::nullopt);

	CComPtr<IVssAsync> pAsync;
	rc = m_pVssObject->PrepareForBackup(&pAsync);
	CHECK_HR_RETURN(rc, "PrepareForBackup", std::nullopt);
	CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "pAsync1->Wait", std::nullopt);

	rc = m_pVssObject->DoSnapshotSet(&pAsync);
	CHECK_HR_RETURN(rc, "DoSnapshotSet", std::nullopt);
	CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "pAsync2->Wait", std::nullopt);

	//rc = m_pVssObject->BackupComplete(&pAsync);
	//CHECK_HR_RETURN(rc, "BackupComplete", std::nullopt); // TODO:: try to figure out why crashed
	//CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "pAsync3->Wait", std::nullopt);
	//std::cout << "bp3" << std::endl;

	std::optional<std::wstring> wSnapshotSetIDStr = VssID2WStr(snapshotSetID);
	std::optional<std::wstring> wSnapshotIDStr = VssID2WStr(snapshotID);
	if (!wSnapshotIDStr || !wSnapshotSetIDStr) {
		// TODO:: fatal error, try throw exception
		return std::nullopt;
	}
	
	SnapshotSetResult result;
	result.m_wSnapshotIDList.emplace_back(wSnapshotIDStr.value());
	result.m_wSnapshotSetID = wSnapshotSetIDStr.value();
	return std::make_optional<SnapshotSetResult>(result);
}

std::optional<SnapshotSetResult> VssClient::CreateSnapshot(const std::string& volumePath)
{
	std::wstring wVolumePath = Utf8ToUtf16(volumePath);
	return CreateSnapshotW(wVolumePath);
}

bool VssClient::DeleteSnapshotW(const std::wstring& wSnapshotID)
{
	LONG lSnapshots = 0;
    VSS_ID idNonDeletedSnapshotID = GUID_NULL;
	std::optional<VSS_ID> snapshotID = VssIDfromWStr(wSnapshotID);
	if (!snapshotID) { /* invalid VSS ID */
		return false;
	}
    HRESULT hr = m_pVssObject->DeleteSnapshots(
        snapshotID.value(), 
        VSS_OBJECT_SNAPSHOT,
        FALSE,
        &lSnapshots,
        &idNonDeletedSnapshotID);
	CHECK_HR_RETURN(hr, "DeleteSnapshots", false);
	return true;
}

bool VssClient::DeleteSnapshot(const std::string& snapshotID)
{
	std::wstring wSnapshotID = Utf8ToUtf16(snapshotID);
	return DeleteSnapshotW(wSnapshotID);
}

std::optional<VssSnapshotProperty> VssClient::GetSnapshotPropertyW(const std::wstring& wSnapshotIDStr)
{
	std::optional<VSS_ID> snapshotID = VssIDfromWStr(wSnapshotIDStr);
	if (!snapshotID) {
		return std::nullopt;
	}

	VSS_SNAPSHOT_PROP snapshotProp;
	HRESULT rc = m_pVssObject->GetSnapshotProperties(snapshotID.value(), &snapshotProp);
	CHECK_HR_RETURN(rc, "GetSnapshotProperties", std::nullopt);
	VssSnapshotProperty property(snapshotProp);
	::VssFreeSnapshotProperties(&snapshotProp);
	return std::make_optional<VssSnapshotProperty>(property);
}

std::optional<VssSnapshotProperty> VssClient::GetSnapshotProperty(const std::string& snapshotIDStr)
{
	const std::wstring wSnapshotIDstr = Utf8ToUtf16(snapshotIDStr);
	return GetSnapshotPropertyW(wSnapshotIDstr);
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
	HRESULT rc = ::CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
    CHECK_HR_RETURN_FALSE(rc, "CoInitialize");
	m_comInitialized = true;
	return true;
}

bool VssClient::Connect()
{
	HRESULT rc = ::CreateVssBackupComponents(&m_pVssObject);
	CHECK_HR_RETURN_FALSE(rc, "CreateVssBackupComponents");

	rc = m_pVssObject->InitializeForBackup();
	CHECK_HR_RETURN_FALSE(rc, "InitializeForBackup");

	rc = m_pVssObject->SetContext(VSS_CTX_BACKUP);
	CHECK_HR_RETURN_FALSE(rc, "SetContext");

	return true;
}

bool VssClient::WaitAndCheckForAsyncOperation(IVssAsync* pAsync)
{
    HRESULT rc = pAsync->Wait();
	CHECK_HR_RETURN_FALSE(rc, "WaitAndCheckForAsyncOperation pAsync->Wait");

    /* Check the result of the asynchronous operation */
    HRESULT hrReturned = S_OK;
    rc = pAsync->QueryStatus(&hrReturned, nullptr);
	CHECK_HR_RETURN_FALSE(rc, "WaitAndCheckForAsyncOperation pAsync->QueryStatus");

    /* Check if the async operation succeeded... */
    CHECK_HR_RETURN_FALSE(hrReturned, "WaitAndCheckForAsyncOperation return false");

	return true;
}

void VssClient::ReleaseResources()
{
	if (m_pVssObject != nullptr) {
		m_pVssObject->Release();
		m_pVssObject = nullptr;
	}
	if (m_comInitialized) {
		::CoUninitialize();
		m_comInitialized = false;
	}
}

#endif