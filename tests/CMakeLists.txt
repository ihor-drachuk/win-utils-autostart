# Fix warning for FetchContent_Declare regarding DOWNLOAD_EXTRACT_TIMESTAMP
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
endif()

if (NOT WIN_UTILS_AUTOSTART_ENABLE_TESTS)
    return()
endif()

include(FetchContent)

if (NOT ${WIN_UTILS_AUTOSTART_GTEST_SEARCH_MODE} STREQUAL "Skip")
    if (${WIN_UTILS_AUTOSTART_GTEST_SEARCH_MODE} STREQUAL "Auto")
        set(WIN_UTILS_AUTOSTART__INTERNAL_GTEST_SEARCH_MODE QUIET)
    else()
        set(WIN_UTILS_AUTOSTART__INTERNAL_GTEST_SEARCH_MODE REQUIRED)
    endif()
endif()

if (NOT TARGET gtest)
    if (NOT ${WIN_UTILS_AUTOSTART_GTEST_SEARCH_MODE} STREQUAL "Skip")
        find_package(GTest ${WIN_UTILS_AUTOSTART__INTERNAL_GTEST_SEARCH_MODE})
    endif()

    if (GTest_FOUND)
        message("GTest found!")
    else()
        message("GTest NOT found! Adding external source...")

        set(INSTALL_GTEST OFF CACHE BOOL "Do not install gtest" FORCE)
        set(INSTALL_GMOCK OFF CACHE BOOL "Do not install gmock" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
        )

        FetchContent_MakeAvailable(googletest)
    endif()
endif()

enable_testing()

add_subdirectory(test)
