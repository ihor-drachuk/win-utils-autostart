/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/win-utils-autostart
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#include <win-utils-autostart/win_utils_autostart.h>

using namespace win_utils_autostart;

// EventLog service is guaranteed to be running and set to autostart on all Windows systems.
static const std::wstring AlwaysRunningService = L"EventLog";
static const std::wstring NonExistentService = L"win-utils-autostart-nonexistent-service-12345";

TEST(ServiceQuery, GetServiceState_EventLog_ReturnsRunning)
{
    auto state = getServiceState(AlwaysRunningService);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(*state, ServiceRunState::Running);
}

TEST(ServiceQuery, IsServiceRunning_EventLog_ReturnsTrue)
{
    auto result = isServiceRunning(AlwaysRunningService);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST(ServiceQuery, IsServiceStartingOrRunning_EventLog_ReturnsTrue)
{
    auto result = isServiceStartingOrRunning(AlwaysRunningService);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST(ServiceQuery, IsServiceAutoStart_EventLog_ReturnsTrue)
{
    auto result = isServiceAutoStart(AlwaysRunningService);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST(ServiceQuery, GetServiceState_NonExistent_ReturnsNullopt)
{
    auto state = getServiceState(NonExistentService);
    EXPECT_FALSE(state.has_value());
}

TEST(ServiceQuery, IsServiceRunning_NonExistent_ReturnsNullopt)
{
    auto result = isServiceRunning(NonExistentService);
    EXPECT_FALSE(result.has_value());
}

TEST(ServiceQuery, IsServiceAutoStart_NonExistent_ReturnsNullopt)
{
    auto result = isServiceAutoStart(NonExistentService);
    EXPECT_FALSE(result.has_value());
}

// --- Service control tests (require admin rights, disabled by default) ---

TEST(DISABLED_ServiceControl, SetServiceState_Start_AlreadyRunning_Succeeds)
{
    EXPECT_TRUE(setServiceState(AlwaysRunningService, ServiceState::Running));
}
