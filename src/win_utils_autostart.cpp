/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <win-utils-autostart/win_utils_autostart.h>

#include <cassert>
#include <array>
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "internal/registry_helpers.h"
#include "internal/elevation_helpers.h"

namespace win_utils_autostart {

namespace {

constexpr wchar_t AutorunRegKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Run)";
constexpr wchar_t StartupApprovedRegKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run)";

// Enabled marker: first byte 0x02, 12 bytes total
constexpr std::array<uint8_t, 12> EnabledMarker  = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Disabled marker: first byte 0x03, 12 bytes total
constexpr std::array<uint8_t, 12> DisabledMarker = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

internal::RegistryScope toRegistryScope(bool systemWide)
{
    return systemWide ? internal::RegistryScope::SystemScope : internal::RegistryScope::UserScope;
}

bool isValidServiceName(const std::wstring& name)
{
    if (name.empty())
        return false;

    for (wchar_t ch : name) {
        if (ch >= L'a' && ch <= L'z') continue;
        if (ch >= L'A' && ch <= L'Z') continue;
        if (ch >= L'0' && ch <= L'9') continue;
        if (ch == L'_' || ch == L'-' || ch == L'.') continue;
        return false;
    }

    return true;
}

} // namespace

// --- Service query ---

std::optional<ServiceRunState> getServiceState(const std::wstring& serviceName)
{
    std::optional<ServiceRunState> result;
    SC_HANDLE schManager {nullptr};
    SC_HANDLE schService {nullptr};
    SERVICE_STATUS_PROCESS ssStatus {};
    DWORD dwBytesNeeded {};
    DWORD savedError {};

    do {
        schManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (!schManager) {
            savedError = GetLastError();
            break;
        }

        schService = OpenServiceW(schManager, serviceName.c_str(), SERVICE_QUERY_STATUS);
        if (!schService) {
            savedError = GetLastError();
            break;
        }

        if (!QueryServiceStatusEx(schService,
                                  SC_STATUS_PROCESS_INFO,
                                  reinterpret_cast<LPBYTE>(&ssStatus),
                                  sizeof(SERVICE_STATUS_PROCESS),
                                  &dwBytesNeeded))
        {
            savedError = GetLastError();
            break;
        }

        result = static_cast<ServiceRunState>(ssStatus.dwCurrentState);
    } while (false);

    if (schService)
        CloseServiceHandle(schService);
    if (schManager)
        CloseServiceHandle(schManager);

    if (!result)
        SetLastError(savedError);

    return result;
}

std::optional<bool> isServiceRunning(const std::wstring& serviceName)
{
    auto state = getServiceState(serviceName);
    if (!state)
        return std::nullopt;
    return *state == ServiceRunState::Running;
}

std::optional<bool> isServiceStartingOrRunning(const std::wstring& serviceName)
{
    auto state = getServiceState(serviceName);
    if (!state)
        return std::nullopt;
    return *state == ServiceRunState::Running ||
           *state == ServiceRunState::StartPending ||
           *state == ServiceRunState::ContinuePending;
}

std::optional<bool> isServiceAutoStart(const std::wstring& serviceName)
{
    SC_HANDLE schSCManager {nullptr};
    SC_HANDLE schService {nullptr};
    LPQUERY_SERVICE_CONFIG lpsc {nullptr};
    DWORD dwBytesNeeded {}, cbBufSize {};
    std::optional<bool> result;
    DWORD savedError {};

    do {
        schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (!schSCManager) {
            savedError = GetLastError();
            break;
        }

        schService = OpenServiceW(schSCManager, serviceName.c_str(), SERVICE_QUERY_CONFIG);
        if (!schService) {
            savedError = GetLastError();
            break;
        }

        if (!QueryServiceConfig(schService, nullptr, 0, &dwBytesNeeded)) {
            auto dwError = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == dwError) {
                cbBufSize = dwBytesNeeded;
                lpsc = static_cast<LPQUERY_SERVICE_CONFIG>(LocalAlloc(LMEM_FIXED, cbBufSize));
                if (!lpsc) {
                    savedError = GetLastError();
                    break;
                }
            } else {
                savedError = dwError;
                break;
            }
        }

        if (!QueryServiceConfig(schService, lpsc, cbBufSize, &dwBytesNeeded)) {
            savedError = GetLastError();
            break;
        }

        result = lpsc->dwStartType != SERVICE_DEMAND_START && lpsc->dwStartType != SERVICE_DISABLED;
    } while (false);

    if (lpsc)
        LocalFree(lpsc);
    if (schService)
        CloseServiceHandle(schService);
    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (!result)
        SetLastError(savedError);

    return result;
}

// --- Service control ---

bool setServiceState(const std::wstring& serviceName, ServiceState state)
{
    SC_HANDLE schManager {nullptr};
    SC_HANDLE schService {nullptr};
    const auto accessMask = [state]() -> DWORD {
        switch (state) {
            case ServiceState::Running: return SERVICE_START | SERVICE_QUERY_STATUS;
            case ServiceState::Stopped: return SERVICE_STOP | SERVICE_QUERY_STATUS;

            case ServiceState::Autostart: [[fallthrough]];
            case ServiceState::Manual:    return SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS;

            case ServiceState::RunningAndAutostart: return SERVICE_START | SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS;
            case ServiceState::StoppedAndManual:    return SERVICE_STOP | SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS;
        }

        assert(false && "Unhandled ServiceState value!");
        return 0;
    }();
    bool success = false;
    DWORD savedError {};

    do {
        schManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (!schManager) {
            savedError = GetLastError();
            break;
        }

        schService = OpenServiceW(schManager, serviceName.c_str(), accessMask);
        if (!schService) {
            savedError = GetLastError();
            break;
        }

        SERVICE_STATUS_PROCESS ssStatus {};
        DWORD dwBytesNeeded {};

        if (!QueryServiceStatusEx(schService,
                                  SC_STATUS_PROCESS_INFO,
                                  reinterpret_cast<LPBYTE>(&ssStatus),
                                  sizeof(SERVICE_STATUS_PROCESS),
                                  &dwBytesNeeded))
        {
            savedError = GetLastError();
            break;
        }

        switch (state) {
            case ServiceState::Running:
                if (ssStatus.dwCurrentState != SERVICE_RUNNING) {
                    if (!StartServiceW(schService, 0, nullptr)) {
                        savedError = GetLastError();
                        break;
                    }
                }
                success = true;
                break;

            case ServiceState::Stopped:
                if (ssStatus.dwCurrentState != SERVICE_STOPPED) {
                    SERVICE_STATUS sStatus {};
                    if (!ControlService(schService, SERVICE_CONTROL_STOP, &sStatus)) {
                        savedError = GetLastError();
                        break;
                    }
                }
                success = true;
                break;

            case ServiceState::Autostart:
                if (!ChangeServiceConfigW(schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE,
                                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
                {
                    savedError = GetLastError();
                    break;
                }
                success = true;
                break;

            case ServiceState::Manual:
                if (!ChangeServiceConfigW(schService, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE,
                                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
                {
                    savedError = GetLastError();
                    break;
                }
                success = true;
                break;

            case ServiceState::RunningAndAutostart:
                if (!ChangeServiceConfigW(schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE,
                                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
                {
                    savedError = GetLastError();
                    break;
                }

                if (ssStatus.dwCurrentState != SERVICE_RUNNING) {
                    if (!StartServiceW(schService, 0, nullptr)) {
                        savedError = GetLastError();
                        break;
                    }
                }
                success = true;
                break;

            case ServiceState::StoppedAndManual:
                if (!ChangeServiceConfigW(schService, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE,
                                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
                {
                    savedError = GetLastError();
                    break;
                }

                if (ssStatus.dwCurrentState != SERVICE_STOPPED) {
                    SERVICE_STATUS sStatus {};
                    if (!ControlService(schService, SERVICE_CONTROL_STOP, &sStatus)) {
                        savedError = GetLastError();
                        break;
                    }
                }
                success = true;
                break;
        }
    } while (false);

    if (schService)
        CloseServiceHandle(schService);
    if (schManager)
        CloseServiceHandle(schManager);

    if (!success)
        SetLastError(savedError);

    return success;
}

bool setServiceStateAsAdmin(const std::wstring& serviceName, ServiceState state)
{
    if (!isValidServiceName(serviceName)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    constexpr wchar_t runObj[] = L"PowerShell";
    constexpr wchar_t start[] = L"Start-Service -Name ";
    constexpr wchar_t stop[] = L"Stop-Service -Name ";
    constexpr wchar_t set[] = L"Set-Service -Name ";
    constexpr wchar_t automatic[] = L" -StartupType Automatic";
    constexpr wchar_t manual[] = L" -StartupType Manual";

    std::wstring param;

    switch (state) {
        case ServiceState::Running:
            param = start + serviceName;
            break;
        case ServiceState::Stopped:
            param = stop + serviceName;
            break;
        case ServiceState::Autostart:
            param = set + serviceName + automatic;
            break;
        case ServiceState::Manual:
            param = set + serviceName + manual;
            break;
        case ServiceState::RunningAndAutostart:
            param = start + serviceName + L"; " + set + serviceName + automatic;
            break;
        case ServiceState::StoppedAndManual:
            param = stop + serviceName + L"; " + set + serviceName + manual;
            break;
    }

    return internal::runElevationToAdmin(runObj, param);
}

bool setServiceStateAuto(const std::wstring& serviceName, ServiceState state)
{
    if (setServiceState(serviceName, state))
        return true;

    return setServiceStateAsAdmin(serviceName, state);
}

// --- Application autostart ---

bool enableAppAutostart(bool systemWide, const std::wstring& appName, const std::wstring& appPath)
{
    auto scope = toRegistryScope(systemWide);

    if (!internal::writeRegistryString(scope, AutorunRegKey, appName, appPath))
        return false;

    std::vector<uint8_t> marker(EnabledMarker.begin(), EnabledMarker.end());
    return internal::writeRegistryBinary(scope, StartupApprovedRegKey, appName, marker);
}

bool disableAppAutostart(bool systemWide, const std::wstring& appName)
{
    auto scope = toRegistryScope(systemWide);

    std::vector<uint8_t> marker(DisabledMarker.begin(), DisabledMarker.end());
    return internal::writeRegistryBinary(scope, StartupApprovedRegKey, appName, marker);
}

bool removeAppAutostart(bool systemWide, const std::wstring& appName)
{
    auto scope = toRegistryScope(systemWide);

    bool ok1 = internal::removeRegistryEntry(scope, AutorunRegKey, appName);
    bool ok2 = internal::removeRegistryEntry(scope, StartupApprovedRegKey, appName);
    return ok1 && ok2;
}

bool isAppAutostartEnabled(bool systemWide, const std::wstring& appName)
{
    auto scope = toRegistryScope(systemWide);

    if (!internal::checkRegistryEntryExists(scope, AutorunRegKey, appName))
        return false;

    auto data = internal::readRegistryBinary(scope, StartupApprovedRegKey, appName);
    if (data.empty())
        return false;

    return data[0] == 0x02; // First byte 0x02 means "Enabled"
}

} // namespace win_utils_autostart
