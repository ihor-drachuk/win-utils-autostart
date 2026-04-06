/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "../../src/internal/registry_helpers.h"

using namespace win_utils_autostart::internal;

namespace {

const std::wstring TestRegKey = LR"(Software\win-utils-autostart-test)";

class RegistryTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        HKEY hkey {};
        if (RegOpenKeyExW(HKEY_CURRENT_USER, TestRegKey.c_str(), 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS) {
            for (const auto& name : { L"test-string", L"test-binary", L"test-remove" }) {
                RegDeleteValueW(hkey, name);
            }
            RegCloseKey(hkey);
        }
        RegDeleteKeyW(HKEY_CURRENT_USER, TestRegKey.c_str());
    }
};

} // namespace

TEST_F(RegistryTest, WriteString_ReadBack_Matches)
{
    const std::wstring value = L"Hello, Registry!";
    ASSERT_TRUE(writeRegistryString(RegistryScope::UserScope, TestRegKey, L"test-string", value));

    auto result = readRegistryString(RegistryScope::UserScope, TestRegKey, L"test-string");
    EXPECT_EQ(result, value);
}

TEST_F(RegistryTest, WriteBinary_ReadBack_Matches)
{
    const std::vector<uint8_t> data = { 0x02, 0x00, 0x00, 0x00, 0xAA, 0xBB };
    ASSERT_TRUE(writeRegistryBinary(RegistryScope::UserScope, TestRegKey, L"test-binary", data));

    auto result = readRegistryBinary(RegistryScope::UserScope, TestRegKey, L"test-binary");
    EXPECT_EQ(result, data);
}

TEST_F(RegistryTest, CheckExists_AfterWrite_ReturnsTrue)
{
    ASSERT_TRUE(writeRegistryString(RegistryScope::UserScope, TestRegKey, L"test-string", L"value"));
    EXPECT_TRUE(checkRegistryEntryExists(RegistryScope::UserScope, TestRegKey, L"test-string"));
}

TEST_F(RegistryTest, CheckExists_Missing_ReturnsFalse)
{
    EXPECT_FALSE(checkRegistryEntryExists(RegistryScope::UserScope, TestRegKey, L"nonexistent-value"));
}

TEST_F(RegistryTest, Remove_AfterWrite_ThenNotExists)
{
    ASSERT_TRUE(writeRegistryString(RegistryScope::UserScope, TestRegKey, L"test-remove", L"to-delete"));
    ASSERT_TRUE(checkRegistryEntryExists(RegistryScope::UserScope, TestRegKey, L"test-remove"));

    ASSERT_TRUE(removeRegistryEntry(RegistryScope::UserScope, TestRegKey, L"test-remove"));
    EXPECT_FALSE(checkRegistryEntryExists(RegistryScope::UserScope, TestRegKey, L"test-remove"));
}

TEST_F(RegistryTest, ReadString_Missing_ReturnsEmpty)
{
    auto result = readRegistryString(RegistryScope::UserScope, TestRegKey, L"nonexistent");
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(GetLastError(), static_cast<DWORD>(ERROR_FILE_NOT_FOUND));
}

TEST_F(RegistryTest, WriteString_OverwriteExisting_Updates)
{
    ASSERT_TRUE(writeRegistryString(RegistryScope::UserScope, TestRegKey, L"test-string", L"first"));
    ASSERT_TRUE(writeRegistryString(RegistryScope::UserScope, TestRegKey, L"test-string", L"second"));

    auto result = readRegistryString(RegistryScope::UserScope, TestRegKey, L"test-string");
    EXPECT_EQ(result, L"second");
}
