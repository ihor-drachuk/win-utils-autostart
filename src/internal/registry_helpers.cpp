/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#include "registry_helpers.h"

#include <cassert>

namespace win_utils_autostart {
namespace internal {

HKEY scope2HKey(RegistryScope scope)
{
    switch (scope) {
        case RegistryScope::SystemScope: return HKEY_LOCAL_MACHINE;
        case RegistryScope::UserScope:   return HKEY_CURRENT_USER;
    }

    assert(false && "Unhandled RegistryScope value!");
    return {};
}

std::wstring readRegistryString(RegistryScope scope, const std::wstring& key, const std::wstring& valueName)
{
    assert(!key.empty());

    DWORD sizeInBytes {};
    DWORD type {};
    DWORD rights = RRF_RT_REG_SZ | RRF_NOEXPAND | RRF_SUBKEY_WOW6464KEY;

    auto rc = RegGetValueW(scope2HKey(scope), key.c_str(), valueName.c_str(), rights, &type, nullptr, &sizeInBytes);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return {};
    }

    std::vector<wchar_t> buffer(sizeInBytes / sizeof(wchar_t));

    rc = RegGetValueW(scope2HKey(scope), key.c_str(), valueName.c_str(), rights, &type, buffer.data(), &sizeInBytes);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return {};
    }

    // Remove null terminator if present
    std::wstring result(buffer.data());
    return result;
}

std::vector<uint8_t> readRegistryBinary(RegistryScope scope, const std::wstring& key, const std::wstring& valueName)
{
    assert(!key.empty());

    DWORD sizeInBytes {};
    DWORD type {};
    DWORD rights = RRF_RT_REG_BINARY | RRF_NOEXPAND | RRF_SUBKEY_WOW6464KEY;

    auto rc = RegGetValueW(scope2HKey(scope), key.c_str(), valueName.c_str(), rights, &type, nullptr, &sizeInBytes);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return {};
    }

    std::vector<uint8_t> buffer(sizeInBytes);

    rc = RegGetValueW(scope2HKey(scope), key.c_str(), valueName.c_str(), rights, &type, buffer.data(), &sizeInBytes);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return {};
    }

    return buffer;
}

bool writeRegistryString(RegistryScope scope, const std::wstring& key, const std::wstring& valueName, const std::wstring& data)
{
    assert(!key.empty());

    HKEY hkey {};
    DWORD rights = KEY_SET_VALUE | KEY_WOW64_64KEY;

    LSTATUS rc = RegCreateKeyExW(scope2HKey(scope),
                                 key.c_str(),
                                 0,
                                 nullptr,
                                 REG_OPTION_NON_VOLATILE,
                                 rights,
                                 nullptr,
                                 &hkey,
                                 nullptr);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return false;
    }

    const BYTE* dataRaw = reinterpret_cast<const BYTE*>(data.c_str());
    DWORD dataSize = static_cast<DWORD>((data.size() + 1) * sizeof(wchar_t));
    rc = RegSetValueExW(hkey, valueName.c_str(), 0, REG_SZ, dataRaw, dataSize);

    auto savedError = static_cast<DWORD>(rc);
    RegCloseKey(hkey);

    if (rc != ERROR_SUCCESS) {
        SetLastError(savedError);
        return false;
    }

    return true;
}

bool writeRegistryBinary(RegistryScope scope, const std::wstring& key, const std::wstring& valueName, const std::vector<uint8_t>& data)
{
    assert(!key.empty());

    HKEY hkey {};
    DWORD rights = KEY_SET_VALUE | KEY_WOW64_64KEY;

    LSTATUS rc = RegCreateKeyExW(scope2HKey(scope),
                                 key.c_str(),
                                 0,
                                 nullptr,
                                 REG_OPTION_NON_VOLATILE,
                                 rights,
                                 nullptr,
                                 &hkey,
                                 nullptr);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return false;
    }

    const BYTE* dataRaw = reinterpret_cast<const BYTE*>(data.data());
    DWORD dataSize = static_cast<DWORD>(data.size());
    rc = RegSetValueExW(hkey, valueName.c_str(), 0, REG_BINARY, dataRaw, dataSize);

    auto savedError = static_cast<DWORD>(rc);
    RegCloseKey(hkey);

    if (rc != ERROR_SUCCESS) {
        SetLastError(savedError);
        return false;
    }

    return true;
}

bool removeRegistryEntry(RegistryScope scope, const std::wstring& key, const std::wstring& valueName)
{
    assert(!key.empty());

    HKEY hkey {};
    DWORD rights = KEY_SET_VALUE | KEY_WOW64_64KEY;

    LSTATUS rc = RegOpenKeyExW(scope2HKey(scope), key.c_str(), 0, rights, &hkey);
    if (rc != ERROR_SUCCESS) {
        SetLastError(static_cast<DWORD>(rc));
        return false;
    }

    rc = RegDeleteValueW(hkey, valueName.c_str());

    auto savedError = static_cast<DWORD>(rc);
    RegCloseKey(hkey);

    if (rc != ERROR_SUCCESS) {
        SetLastError(savedError);
        return false;
    }

    return true;
}

bool checkRegistryEntryExists(RegistryScope scope, const std::wstring& key, const std::wstring& valueName)
{
    assert(!key.empty());

    DWORD sizeInBytes {};
    DWORD type {};
    DWORD rights = RRF_RT_ANY | RRF_NOEXPAND | RRF_SUBKEY_WOW6464KEY;

    auto rc = RegGetValueW(scope2HKey(scope), key.c_str(), valueName.c_str(), rights, &type, nullptr, &sizeInBytes);
    if (rc == ERROR_SUCCESS)
        return true;

    SetLastError(static_cast<DWORD>(rc));
    return false;
}

} // namespace internal
} // namespace win_utils_autostart
