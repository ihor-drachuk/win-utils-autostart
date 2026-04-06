/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <win-utils-autostart/win_utils_autostart.h>
#include "../../src/internal/registry_helpers.h"

using namespace win_utils_autostart;
using namespace win_utils_autostart::internal;

namespace {

const std::wstring TestAppName = L"win-utils-autostart-test-app";
const std::wstring AutorunRegKey = LR"(Software\Microsoft\Windows\CurrentVersion\Run)";
const std::wstring StartupApprovedRegKey = LR"(Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run)";

class AutostartTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        removeRegistryEntry(RegistryScope::UserScope, AutorunRegKey, TestAppName);
        removeRegistryEntry(RegistryScope::UserScope, StartupApprovedRegKey, TestAppName);
    }
};

} // namespace

TEST_F(AutostartTest, Enable_ThenIsEnabled)
{
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, LR"(C:\test\app.exe)"));
    EXPECT_TRUE(isAppAutostartEnabled(false, TestAppName));
}

TEST_F(AutostartTest, Enable_ThenDisable_ThenIsNotEnabled)
{
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, LR"(C:\test\app.exe)"));
    ASSERT_TRUE(disableAppAutostart(false, TestAppName));
    EXPECT_FALSE(isAppAutostartEnabled(false, TestAppName));
}

TEST_F(AutostartTest, IsEnabled_NeverSet_ReturnsFalse)
{
    EXPECT_FALSE(isAppAutostartEnabled(false, TestAppName));
}

TEST_F(AutostartTest, Enable_VerifyRegistryPath)
{
    const std::wstring appPath = LR"(C:\test\app.exe)";
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, appPath));

    auto storedPath = readRegistryString(RegistryScope::UserScope, AutorunRegKey, TestAppName);
    EXPECT_EQ(storedPath, appPath);

    auto approvedData = readRegistryBinary(RegistryScope::UserScope, StartupApprovedRegKey, TestAppName);
    ASSERT_FALSE(approvedData.empty());
    EXPECT_EQ(approvedData[0], 0x02);
}

TEST_F(AutostartTest, Disable_VerifyRegistryMarker)
{
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, LR"(C:\test\app.exe)"));
    ASSERT_TRUE(disableAppAutostart(false, TestAppName));

    auto approvedData = readRegistryBinary(RegistryScope::UserScope, StartupApprovedRegKey, TestAppName);
    ASSERT_FALSE(approvedData.empty());
    EXPECT_EQ(approvedData[0], 0x03);
}

TEST_F(AutostartTest, Enable_UpdateExisting_OverwritesPath)
{
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, LR"(C:\old\app.exe)"));
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, LR"(C:\new\app.exe)"));

    auto storedPath = readRegistryString(RegistryScope::UserScope, AutorunRegKey, TestAppName);
    EXPECT_EQ(storedPath, LR"(C:\new\app.exe)");
    EXPECT_TRUE(isAppAutostartEnabled(false, TestAppName));
}

TEST_F(AutostartTest, Remove_CleansUpBothKeys)
{
    ASSERT_TRUE(enableAppAutostart(false, TestAppName, LR"(C:\test\app.exe)"));
    ASSERT_TRUE(removeAppAutostart(false, TestAppName));

    EXPECT_FALSE(checkRegistryEntryExists(RegistryScope::UserScope, AutorunRegKey, TestAppName));
    EXPECT_FALSE(checkRegistryEntryExists(RegistryScope::UserScope, StartupApprovedRegKey, TestAppName));
    EXPECT_FALSE(isAppAutostartEnabled(false, TestAppName));
}
