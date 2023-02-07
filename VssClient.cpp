#include "VssClient.h"

#ifdef WIN32
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
#include <locale>
#include <codecvt>
#endif

using namespace std;

namespace {
#ifdef WIN32
constexpr auto VOLUME_BUFFER_MAX_LEN = MAX_PATH;
constexpr auto VOLUME_PATH_MAX_LEN = MAX_PATH + 1;
constexpr auto DEVICE_BUFFER_MAX_LEN = MAX_PATH;
#endif
}

#define CHECK_HR_RETURN(HR, FUNC, RET) \
	do { \
		_com_error err(HR); \
		if (HR != S_OK) { \
			fprintf(stderr, "Failed to call " ## FUNC ## ", %s\n", err.ErrorMessage()); \
			return RET; \
		}
	} \
	while (0)

namespace Win32VSSWrapper {


#ifdef WIN32
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
#endif


VssSnapshotProperty::VssSnapshotProperty(const std::VSS_SNAPSHOT_PROP& prop)
{
	m_snapshotID = prop.m_SnapshotId;
	m_shapshotSetID = prop.m_SnapshotSetId;
	m_snapshotsCount = prop.m_lSnapshotsCount;
	m_wSnapshotDeviceObject = prop.m_pwszSnapshotDeviceObject;
	m_wOriginVolumeName = prop.m_pwszOriginalVolumeName;
	m_wOriginatingMachine = prop.m_pwszOriginatingMachine;
	m_wServiceMachine = prop.m_pwszServiceMachine;
	m_wExposedName = prop.m_pwszExposedName;
	m_wExposedName = m_prop.m_pwszExposedPath;
	m_providerID = prop.m_ProviderId;
	m_napshotAttributes = prop.m_lSnapshotAttributes;
	m_createTime = prop.m_tsCreationTimestamp;
	m_status = prop.m_eStatus;
}

void VssClient::CreateSnapshotW(const std::vector<std::wstring& wVolumePath)
{

}

void VssClient::CreateSnapshotW(const std::wstring& wVolumePath)
{
	VSS_ID snapshot_id;
	HRESULT rc = m_components->AddToSnapshotSetW(wVolumePath.c_str(), GUID_NULL, &snapshot_id);
    CHECK_HR_RETURN(rc, "AddToSnapshotSet", -1);

	/* Generate Snapshot */
	rc = components->SetBackupState(true, false, VSS_BT_FULL, false);
	CHECK_HR_RETURN(rc, "SetBackupState", -1);

	rc = components->PrepareForBackup(&async);
	CHECK_HR_RETURN(rc, "PrepareForBackup", -1);
	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);

	rc = components->DoSnapshotSet(&async);
	CHECK_HR_RETURN(rc, "DoSnapshotSet", -1);
	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);
}

VssSnapshotProperty GetSnapshotProperty(const VSS_ID& snapshotID)
{
	VSS_SNAPSHOT_PROP snapshotProp;
	rc = m_components->GetSnapshotProperties(snapshotID, &snapshotProp);
	CHECK_HR_RETURN(rc, "GetSnapshotProperties", -1);
	VssSnapshotProperty prop(snapshotProp);
	::VssFreeSnapshotProperties(snapshotProp);
	return prop;
}

void VssClient::QuerySnapshotInfo(VSS_ID)
{
	VSS_SNAPSHOT_PROP 
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
void VssClient::Init()
{
	HRESULT rc = ::CoInitialize(nullptr);
    CHECK_HR_RETURN(rc, "CoInitialize", -1);
}

void VssClient::Connect()
{
	rc = ::CreateVssBackupComponents(&m_components);
	CHECK_HR_RETURN(rc, "CreateVssBackupComponents", -1);

	rc = m_components->InitializeForBackup();
	CHECK_HR_RETURN(rc, "InitializeForBackup", -1);

	IVssAsync *async;
	rc = m_components->GatherWriterMetadata(&async);
	CHECK_HR_RETURN(rc, "GatherWriterMetadata", -1);
	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);

	rc = m_components->SetContext(VSS_CTX_BACKUP);
	CHECK_HR_RETURN(rc, "SetContext", -1);

	VSS_ID snapshot_set_id;
	rc = m_components->StartSnapshotSet(&snapshot_set_id);
	CHECK_HR_RETURN(rc, "StartSnapshotSet", -1);	
}

void VssClient::ReleaseResources()
{
	if (m_components != nullptr) {
		HRESULT rc = components->BackupComplete(&m_async);
		CHECK_HR_RETURN(rc, "BackupComplete", -1);
		
		HRESULT rc = async->Wait();
		CHECK_HR_RETURN(rc, "async->Wait", -1);

		m_components->Release();
		::CoUninitialize();
		m_components = nullptr;
	}
}

}