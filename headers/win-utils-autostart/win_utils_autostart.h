/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <optional>
#include <string>

namespace win_utils_autostart {

enum class ServiceState {
    Running,
    Stopped,

    Autostart,
    Manual,

    RunningAndAutostart,
    StoppedAndManual
};

enum class ServiceRunState {
    Stopped = 1, // match SERVICE_STOPPED
    StartPending,
    StopPending,
    Running,
    ContinuePending,
    PausePending,
    Paused
};

// --- Service query (read-only, no admin rights needed) ---
std::optional<bool> isServiceRunning(const std::wstring& serviceName);
std::optional<bool> isServiceStartingOrRunning(const std::wstring& serviceName);
std::optional<bool> isServiceAutoStart(const std::wstring& serviceName);
std::optional<ServiceRunState> getServiceState(const std::wstring& serviceName);

// --- Service control (may require admin rights) ---
bool setServiceState(const std::wstring& serviceName, ServiceState state);
bool setServiceStateAsAdmin(const std::wstring& serviceName, ServiceState state);
bool setServiceStateAuto(const std::wstring& serviceName, ServiceState state);

// --- Application autostart (registry-based) ---
//     systemWide: true = HKLM (all users, requires admin), false = HKCU (current user)
bool enableAppAutostart(bool systemWide, const std::wstring& appName, const std::wstring& appPath);
bool disableAppAutostart(bool systemWide, const std::wstring& appName);
bool removeAppAutostart(bool systemWide, const std::wstring& appName);
bool isAppAutostartEnabled(bool systemWide, const std::wstring& appName);

} // namespace win_utils_autostart
