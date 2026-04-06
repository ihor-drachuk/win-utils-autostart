cmake_minimum_required(VERSION 3.16)
project(win-utils-autostart LANGUAGES CXX)

option(WIN_UTILS_AUTOSTART_ENABLE_TESTS "win-utils-autostart: Enable tests" OFF)

set(WIN_UTILS_AUTOSTART_GTEST_SEARCH_MODE "Auto" CACHE STRING "win-utils-autostart: Set GTest search mode")
set_property(CACHE WIN_UTILS_AUTOSTART_GTEST_SEARCH_MODE PROPERTY STRINGS "Auto" "Force" "Skip")

# Always optimize, even in Debug — Debug only differs by runtime (/MDd vs /MD).
if(MSVC)
    string(REPLACE "/Od"   "/O2" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Ob0"  ""    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" ""    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi"   ""    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Z7"   ""    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h headers/*.h)

add_library(win-utils-autostart STATIC ${SOURCES})
target_include_directories(win-utils-autostart PUBLIC headers)
target_compile_features(win-utils-autostart PUBLIC cxx_std_17)

target_link_libraries(win-utils-autostart PRIVATE advapi32 shell32)

# Tune flags
target_link_options(win-utils-autostart PRIVATE "/ignore:4221")
set_target_properties(win-utils-autostart PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
target_compile_options(win-utils-autostart PRIVATE "/W4" "/WX")

enable_testing()
add_subdirectory(tests)
