/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <string>

namespace win_utils_autostart {
namespace internal {

bool isProcessAdmin();
bool runElevationToAdmin(const std::wstring& runObj, const std::wstring& param);

} // namespace internal
} // namespace win_utils_autostart
