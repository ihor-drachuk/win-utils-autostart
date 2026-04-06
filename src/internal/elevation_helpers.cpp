/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "elevation_helpers.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

namespace win_utils_autostart {
namespace internal {

bool isProcessAdmin()
{
    bool fRet = false;
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}

bool runElevationToAdmin(const std::wstring& runObj, const std::wstring& param)
{
    std::wstring action = isProcessAdmin() ? L"open" : L"runas";

    SHELLEXECUTEINFOW sei {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    sei.lpVerb = action.c_str();
    sei.lpFile = runObj.c_str();
    sei.lpParameters = param.c_str();
    sei.nShow = SW_HIDE;

    if (!ShellExecuteExW(&sei))
        return false;

    if (!sei.hProcess)
        return false;

    WaitForSingleObject(sei.hProcess, INFINITE);

    DWORD exitCode {};
    bool ok = GetExitCodeProcess(sei.hProcess, &exitCode) && exitCode == 0;

    CloseHandle(sei.hProcess);
    return ok;
}

} // namespace internal
} // namespace win_utils_autostart
