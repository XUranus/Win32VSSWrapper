#include "VssClient.h"

#ifdef WIN32
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
#include <locale>
#include <codecvt>
#include <optional>
#include <atlbase.h>
#include <cstdio>

using namespace std;
using namespace Win32VSSWrapper;

#define CHECK_HR_RETURN(HR, FUNC, RET) \
    do { \
        _com_error err(HR); \
        if (HR != S_OK) { \
            ::fprintf(stderr, "HRESULT Return FAILED, Function: " ## FUNC ## ", Error: %s\n", err.ErrorMessage()); \
            return RET; \
        } \
    } \
    while (0)

#define CHECK_BOOL_RETURN(BOOLVALUE, FUNC, RET) \
    do { \
        if ((!BOOLVALUE)) { \
            ::fprintf(stderr, "Boolean Return False, Function: " ## FUNC ## "\n"); \
            return RET; \
        } \
    } \
    while (0)

std::optional<std::wstring> VssID2WStr(const VSS_ID& vssID)
{
    LPOLESTR wVssIDBuf = nullptr;
    CAutoComPointer ptrAutoCleanUp(wVssIDBuf);
    HRESULT hr = ::StringFromIID(vssID, &wVssIDBuf);
    if (FAILED(hr)) {
        return std::nullopt;
    }
    std::wstring wVssIDStr(wVssIDBuf);
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
    if (prop.m_pwszSnapshotDeviceObject) {
        m_wSnapshotDeviceObject = prop.m_pwszSnapshotDeviceObject;
    }
    if (prop.m_pwszOriginalVolumeName) {
        m_wOriginVolumeName = prop.m_pwszOriginalVolumeName;
    }
    if (prop.m_pwszOriginatingMachine) {
        m_wOriginatingMachine = prop.m_pwszOriginatingMachine;
    }
    if (prop.m_pwszServiceMachine) {
        m_wServiceMachine = prop.m_pwszServiceMachine;
    }
    if (prop.m_pwszExposedName) {
        m_wExposedName = prop.m_pwszExposedName;
    }
    if (prop.m_pwszExposedPath) {
        m_wExposedPath = prop.m_pwszExposedPath;
    }
    m_providerID = prop.m_ProviderId;
    m_snapshotAttributes = prop.m_lSnapshotAttributes;
    m_createTime = prop.m_tsCreationTimestamp;
    m_status = prop.m_eStatus;
}

uint64_t VssSnapshotProperty::SnapshotsCount() const 
{
    return m_snapshotsCount;
}

uint64_t VssSnapshotProperty::CreateTime() const
{
    return m_createTime;
}

VSS_SNAPSHOT_STATE VssSnapshotProperty::Status() const
{
    return m_status;
}

uint64_t VssSnapshotProperty::SnapshotAttributes()const
{
    return m_snapshotAttributes;
}

bool VssSnapshotProperty::IsClientAccessible() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_CLIENT_ACCESSIBLE) != 0;
}

bool VssSnapshotProperty::IsExposedLocally() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY) != 0;
}

bool VssSnapshotProperty::IsExposedRemotely() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY) != 0;
}

bool VssSnapshotProperty::IsTransportable() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_TRANSPORTABLE) != 0;
}

bool VssSnapshotProperty::IsNoAutoRelease() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_NO_AUTO_RELEASE) != 0;
}

bool VssSnapshotProperty::IsPersistent() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_PERSISTENT) != 0;
}

bool VssSnapshotProperty::IsHardwareAssisted() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_HARDWARE_ASSISTED) != 0;
}

bool VssSnapshotProperty::IsNoWriters() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_NO_WRITERS) != 0;
}

bool VssSnapshotProperty::IsImported() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_IMPORTED) != 0;
}

bool VssSnapshotProperty::IsPlex() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_PLEX) != 0;
}

bool VssSnapshotProperty::IsDifferential() const
{
    return (m_snapshotAttributes & VSS_VOLSNAP_ATTR_DIFFERENTIAL) != 0;
}

/* VssSnapshotProperty API for UTF-16 */
std::wstring VssSnapshotProperty::SnapshotIDW() const
{
    return VssID2WStr(m_snapshotID).value();
}

std::wstring VssSnapshotProperty::SnapshotSetIDW() const
{
    return VssID2WStr(m_shapshotSetID).value();
}

std::wstring VssSnapshotProperty::SnapshotDeviceObjectW() const
{
    return m_wSnapshotDeviceObject;
}

std::wstring VssSnapshotProperty::OriginVolumeNameW() const
{
    return m_wOriginVolumeName;
}

std::wstring VssSnapshotProperty::OriginatingMachineW() const
{
    return m_wOriginatingMachine;
}

std::wstring VssSnapshotProperty::ServiceMachineW() const
{
    return m_wServiceMachine;
}

std::wstring VssSnapshotProperty::ExposedNameW() const
{
    return m_wExposedName;
}

std::wstring VssSnapshotProperty::ExposedPathW() const
{
    return m_wExposedPath;
}

std::wstring VssSnapshotProperty::ProviderIDW() const
{
    return VssID2WStr(m_providerID).value();
}

/* VssSnapshotProperty API for UTF-8 */
std::string VssSnapshotProperty::SnapshotID() const
{
    return Utf16ToUtf8(SnapshotIDW());
}

std::string VssSnapshotProperty::SnapshotSetID() const
{
    return Utf16ToUtf8(SnapshotSetIDW());
}

std::string VssSnapshotProperty::ProviderID() const
{
    return Utf16ToUtf8(ProviderIDW());
}

std::string VssSnapshotProperty::SnapshotDeviceObject() const
{
    return Utf16ToUtf8(SnapshotDeviceObjectW());
}

std::string VssSnapshotProperty::OriginVolumeName() const
{
    return Utf16ToUtf8(OriginVolumeNameW());
}

std::string VssSnapshotProperty::OriginatingMachine() const
{
    return Utf16ToUtf8(OriginatingMachineW());
}

std::string VssSnapshotProperty::ServiceMachine() const
{
    return Utf16ToUtf8(ServiceMachineW());
}

std::string VssSnapshotProperty::ExposedName() const
{
    return Utf16ToUtf8(ExposedNameW());
}

std::string VssSnapshotProperty::ExposedPath() const
{
    return Utf16ToUtf8(ExposedPathW());
}

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

std::optional<SnapshotSetResult> VssClient::CreateSnapshotsW(const std::vector<std::wstring>& wVolumePathList)
{
    InitializeBackupContect(VSS_CTX_APP_ROLLBACK);
    SnapshotSetResult result;
    /* no need to call GatherWriterMetadata due to no writers involved */
    VSS_ID snapshotSetID;
    HRESULT hr = m_pVssObject->StartSnapshotSet(&snapshotSetID);
    CHECK_HR_RETURN(hr, "StartSnapshotSet", std::nullopt);

    result.m_wSnapshotSetID = VssID2WStr(snapshotSetID).value();

    for (const std::wstring& wVolumePath: wVolumePathList) {
        VSS_ID snapshotID;
        WCHAR volume[MAX_PATH] = { L'\0' };
        wcscpy_s(volume, MAX_PATH, wVolumePath.c_str());
        hr = m_pVssObject->AddToSnapshotSet(volume, GUID_NULL, &snapshotID);
        CHECK_HR_RETURN(hr, "AddToSnapshotSet", std::nullopt);
        result.m_wSnapshotIDList.emplace_back(VssID2WStr(snapshotID).value());
    }
    CHECK_BOOL_RETURN(PrepareForBackupSync(), "PrepareForBackupSync", std::nullopt);
    CHECK_BOOL_RETURN(DoSnapshotSetSync(), "DoSnapshotSetSync", std::nullopt);

    /* no need to call BackupComplete due to no writers involved */
    return std::make_optional<SnapshotSetResult>(result);
}

std::optional<SnapshotSetResult> VssClient::CreateSnapshots(const std::vector<std::string>& volumePathList)
{
    std::vector<std::wstring> wVolumePathList;
    for (const std::string& volumePath: volumePathList) {
        wVolumePathList.emplace_back(Utf8ToUtf16(volumePath));
    }
    return CreateSnapshotsW(wVolumePathList);
}

bool VssClient::DeleteSnapshotW(const std::wstring& wSnapshotID)
{
    InitializeBackupContect(VSS_CTX_ALL);
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

bool VssClient::DeleteSnapshotSetW(const std::wstring& wSnapshotSetID)
{
    InitializeBackupContect(VSS_CTX_ALL);
    LONG lSnapshots = 0;
    VSS_ID idNonDeletedSnapshotID = GUID_NULL;
    std::optional<VSS_ID> snapshotSetID = VssIDfromWStr(wSnapshotSetID);
    if (!snapshotSetID) { /* invalid VSS ID */
        return false;
    }
    HRESULT hr = m_pVssObject->DeleteSnapshots(
        snapshotSetID.value(), 
        VSS_OBJECT_SNAPSHOT_SET,
        FALSE,
        &lSnapshots,
        &idNonDeletedSnapshotID);
    CHECK_HR_RETURN(hr, "DeleteSnapshots", false);
    return true;
}

bool VssClient::DeleteSnapshotSet(const std::string& snapshotSetID)
{
    return DeleteSnapshotW(Utf8ToUtf16(snapshotSetID));
}

bool VssClient::DeleteAllSnapshots()
{
    std::vector<VssSnapshotProperty> properties = QueryAllSnapshots();
    bool ret = true;
    for (const VssSnapshotProperty& property: properties) {
        if (!DeleteSnapshot(property.SnapshotID())) {
            ret = false; /* delete as many as possible */
        }
    }
    return ret;
}

std::optional<VssSnapshotProperty> VssClient::GetSnapshotPropertyW(const std::wstring& wSnapshotIDStr)
{
    InitializeBackupContect(VSS_CTX_ALL);
    std::optional<VSS_ID> snapshotID = VssIDfromWStr(wSnapshotIDStr);
    if (!snapshotID) {
        return std::nullopt;
    }

    VSS_SNAPSHOT_PROP snapshotProp;
    HRESULT hr = m_pVssObject->GetSnapshotProperties(snapshotID.value(), &snapshotProp);
    CHECK_HR_RETURN(hr, "GetSnapshotProperties", std::nullopt);
    VssSnapshotProperty property(snapshotProp);
    ::VssFreeSnapshotProperties(&snapshotProp);
    return std::make_optional<VssSnapshotProperty>(property);
}

std::optional<VssSnapshotProperty> VssClient::GetSnapshotProperty(const std::string& snapshotIDStr)
{
    const std::wstring wSnapshotIDstr = Utf8ToUtf16(snapshotIDStr);
    return GetSnapshotPropertyW(wSnapshotIDstr);
}

std::vector<VssSnapshotProperty> VssClient::QuerySnapshotSetW(const std::wstring& wSnapshotSetID)
{
    std::vector<VssSnapshotProperty> properties = QueryAllSnapshots();
    std::vector<VssSnapshotProperty> results;
    for (const VssSnapshotProperty& property: properties) {
        if (property.SnapshotSetIDW() == wSnapshotSetID) {
            results.emplace_back(property);
        }
    }
    return results;
}

std::vector<VssSnapshotProperty> VssClient::QuerySnapshotSet(const std::string& snapshotSetID)
{
    return QuerySnapshotSetW(Utf8ToUtf16(snapshotSetID));
}

std::vector<VssSnapshotProperty> VssClient::QueryAllSnapshots()
{
    InitializeBackupContect(VSS_CTX_ALL);
    std::vector<VssSnapshotProperty> results;
    CComPtr<IVssEnumObject> pIEnumSnapshots;
    HRESULT hr = m_pVssObject->Query(
        GUID_NULL, 
        VSS_OBJECT_NONE, 
        VSS_OBJECT_SNAPSHOT, 
        &pIEnumSnapshots
    );
    CHECK_HR_RETURN(hr, "Query", results);
    /* Enumerate all shadow copies. */
    VSS_OBJECT_PROP prop;
    VSS_SNAPSHOT_PROP& snap = prop.Obj.Snap;

    while (true)
    {
        ULONG ulFetched;
        hr = pIEnumSnapshots->Next(1, &prop, &ulFetched);
        /* We reached the end of list */
        if (FAILED(hr) || ulFetched == 0) {
            break;
        }
        results.emplace_back(snap);
        ::VssFreeSnapshotProperties(&snap);
    }
    return results;
}

bool VssClient::ExposeSnapshotLocallyW(const std::wstring& wSnapshotID, const std::wstring& wPath)
{
    InitializeBackupContect(VSS_CTX_ALL);
    std::optional<VssSnapshotProperty> property = GetSnapshotPropertyW(wSnapshotID);
    /* invalid snapshot ID */
    if (!property) {
        return false;
    }
    /* client accessible snapshot cannot be exposed locally */
    if (property->IsClientAccessible() ||
        !property->ExposedNameW().empty() ||
        !property->ExposedPathW().empty()) {
        return false;
    }
    std::wstring wMountPath = wPath; /* final mount path */
    if (!wMountPath.empty() && wMountPath.back() != L'\\') {
        wMountPath.push_back(L'\\');
    }
    LPWSTR pwszExposed = nullptr;
    VSS_ID vssID = VssIDfromWStr(wSnapshotID).value();
    HRESULT hr = m_pVssObject->ExposeSnapshot(vssID,nullptr,VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY, (VSS_PWSZ)wMountPath.c_str(), &pwszExposed);
    CHECK_HR_RETURN(hr, "ExposeSnapshot", false);
    
    return true;
}

bool VssClient::ExposeSnapshotLocally(const std::string& snapshotID, const std::string& path)
{
    return ExposeSnapshotLocallyW(Utf8ToUtf16(snapshotID), Utf8ToUtf16(path));
}

VssClient::VssClient()
{
    InitializeCom();
}

VssClient::~VssClient()
{
    if (m_pVssObject != nullptr) {
        m_pVssObject->Release();
        m_pVssObject = nullptr;
    }
    UninitializeCom();
}

/* initialzie COM */
bool VssClient::InitializeCom()
{
    if (m_comInitialized) {
        return true;
    }
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
    CHECK_HR_RETURN(hr, "CoInitializeEx", false);
    m_comInitialized = true;
    hr = CoInitializeSecurity(
        NULL,                           //  Allow *all* VSS writers to communicate back!
        -1,                             //  Default COM authentication service
        NULL,                           //  Default COM authorization service
        NULL,                           //  reserved parameter
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  //  Strongest COM authentication level
        RPC_C_IMP_LEVEL_IMPERSONATE,    //  Minimal impersonation abilities 
        NULL,                           //  Default COM authentication settings
        EOAC_DYNAMIC_CLOAKING,          //  Cloaking
        NULL                            //  Reserved parameter
    );
    CHECK_HR_RETURN(hr, "CoInitializeSecurity", false);
    return true;
}

void VssClient::UninitializeCom()
{
    if (!m_comInitialized) {
        return;
    }
    ::CoUninitialize();
    m_comInitialized = false;
}

bool VssClient::InitializeBackupContect(const VSS_SNAPSHOT_CONTEXT& context)
{
    CHECK_BOOL_RETURN(InitializeVssComponent(), "InitializeVssComponent", false);

    HRESULT hr = m_pVssObject->InitializeForBackup();
    CHECK_HR_RETURN(hr, "InitializeForBackup", false);

    hr = m_pVssObject->SetContext(context);
    CHECK_HR_RETURN(hr, "SetContext", false);

    hr = m_pVssObject->SetBackupState(true, false, VSS_BT_FULL, false);
    CHECK_HR_RETURN(hr, "SetBackupState", false);

    return true;
}

bool VssClient::WaitAndCheckForAsyncOperation(IVssAsync* pAsync)
{
    HRESULT hr = pAsync->Wait();
    CHECK_HR_RETURN(hr, "WaitAndCheckForAsyncOperation pAsync->Wait", false);

    /* Check the result of the asynchronous operation */
    HRESULT hrReturned = S_OK;
    hr = pAsync->QueryStatus(&hrReturned, nullptr);
    CHECK_HR_RETURN(hr, "WaitAndCheckForAsyncOperation pAsync->QueryStatus", false);

    /* Check if the async operation succeeded... */
    if (hrReturned != VSS_S_ASYNC_FINISHED) {
        return false;
    }
    return true;
}

bool VssClient::PrepareForBackupSync()
{
    CComPtr<IVssAsync> pAsync;
    HRESULT hr = m_pVssObject->PrepareForBackup(&pAsync);
    CHECK_HR_RETURN(hr, "PrepareForBackup", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "PrepareForBackup Wait", false);
    return true;
}

bool VssClient::DoSnapshotSetSync()
{
    CComPtr<IVssAsync> pAsync;
    HRESULT hr = m_pVssObject->DoSnapshotSet(&pAsync);
    CHECK_HR_RETURN(hr, "DoSnapshotSet", false);
    CHECK_BOOL_RETURN(WaitAndCheckForAsyncOperation(pAsync), "DoSnapshotSet Wait", false);
    return true;
}

bool VssClient::InitializeVssComponent()
{
    if (m_pVssObject != nullptr) {
        m_pVssObject->Release();
        m_pVssObject = nullptr;
    }
    HRESULT hr = ::CreateVssBackupComponents(&m_pVssObject);
    CHECK_HR_RETURN(hr, "CreateVssBackupComponents", false);
    return true;
}

#endif