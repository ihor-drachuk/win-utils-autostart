/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace win_utils_autostart {
namespace internal {

enum class RegistryScope {
    SystemScope,
    UserScope
};

HKEY scope2HKey(RegistryScope scope);

std::wstring readRegistryString(RegistryScope scope, const std::wstring& key, const std::wstring& valueName);
std::vector<uint8_t> readRegistryBinary(RegistryScope scope, const std::wstring& key, const std::wstring& valueName);

bool writeRegistryString(RegistryScope scope, const std::wstring& key, const std::wstring& valueName, const std::wstring& data);
bool writeRegistryBinary(RegistryScope scope, const std::wstring& key, const std::wstring& valueName, const std::vector<uint8_t>& data);

bool removeRegistryEntry(RegistryScope scope, const std::wstring& key, const std::wstring& valueName);
bool checkRegistryEntryExists(RegistryScope scope, const std::wstring& key, const std::wstring& valueName);

} // namespace internal
} // namespace win_utils_autostart
