<p align="center">
  <h1 align="center">win-utils-autostart</h1>
  <p align="center">
    <strong>Windows service management and application autostart utilities</strong>
  </p>
  <p align="center">
    Query/control Windows services, manage application autostart via registry
  </p>
</p>

<p align="center">
  <a href="https://github.com/ihor-drachuk/win-utils-autostart/blob/master/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++ Standard">
  <img src="https://img.shields.io/badge/Platform-Windows-blueviolet.svg" alt="Platform">
</p>

---

A lightweight C++17 static library for Windows service management and application autostart control. No dependencies beyond the Windows SDK — no Qt, no Boost, no external frameworks.

---

## Key Features

- **Service queries** — check if a service is running, starting, or set to autostart
- **Service control** — start, stop, change startup type (with optional UAC elevation fallback)
- **Application autostart** — enable/disable/remove/check app autostart via Windows Registry (`Run` + `StartupApproved`)
- **Self-contained** — only depends on Windows SDK (`advapi32`, `shell32`)
- **Error-transparent** — on failure, `GetLastError()` returns the original WinAPI error code (preserved across internal cleanup)

---

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [API Reference](#api-reference)
- [Building](#building)
- [License](#license)

---

## Requirements

- **CMake** 3.16+
- **C++17** compatible compiler (MSVC recommended)
- **Windows SDK**

---

## Installation

### Option #1: Auto-download

```cmake
include(FetchContent)
FetchContent_Declare(win-utils-autostart
    GIT_REPOSITORY https://github.com/ihor-drachuk/win-utils-autostart.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(win-utils-autostart)

target_link_libraries(YourProject PRIVATE win-utils-autostart)
```

### Option #2: As submodule

```bash
git submodule add https://github.com/ihor-drachuk/win-utils-autostart.git third-party/win-utils-autostart
```

```cmake
add_subdirectory(third-party/win-utils-autostart)
target_link_libraries(YourProject PRIVATE win-utils-autostart)
```

---

## API Reference

All functions are in namespace `win_utils_autostart`.

### Service Query (read-only, no admin rights)

| Function | Returns | Description |
|----------|---------|-------------|
| `getServiceState(name)` | `optional<ServiceRunState>` | Current service run state |
| `isServiceRunning(name)` | `optional<bool>` | Whether service is running |
| `isServiceStartingOrRunning(name)` | `optional<bool>` | Running, starting, or continuing |
| `isServiceAutoStart(name)` | `optional<bool>` | Whether service is set to auto-start |

### Service Control (may require admin)

| Function | Returns | Description |
|----------|---------|-------------|
| `setServiceState(name, state)` | `bool` | Start/stop/change config directly |
| `setServiceStateAsAdmin(name, state)` | `bool` | Via PowerShell with UAC elevation (blocks until complete) |
| `setServiceStateAuto(name, state)` | `bool` | Try direct, fallback to admin elevation on failure |

### Application Autostart (registry-based)

| Function | Returns | Description |
|----------|---------|-------------|
| `enableAppAutostart(systemWide, name, path)` | `bool` | Register app for autostart |
| `disableAppAutostart(systemWide, name)` | `bool` | Soft-disable (sets disabled marker, preserves Run entry) |
| `removeAppAutostart(systemWide, name)` | `bool` | Full removal of both Run and StartupApproved entries |
| `isAppAutostartEnabled(systemWide, name)` | `bool` | Check if app autostart is enabled |

The `systemWide` parameter: `true` = HKLM (all users, requires admin), `false` = HKCU (current user).

### Enums

- **`ServiceState`**: `Running`, `Stopped`, `Autostart`, `Manual`, `RunningAndAutostart`, `StoppedAndManual`
- **`ServiceRunState`**: `Stopped`, `StartPending`, `StopPending`, `Running`, `ContinuePending`, `PausePending`, `Paused`

### Error Handling

On failure, functions return `false` / `std::nullopt`. Call `GetLastError()` immediately after to get the original WinAPI error code — it is preserved across internal cleanup operations (handle closing, memory freeing).

---

## Building

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `WIN_UTILS_AUTOSTART_ENABLE_TESTS` | `OFF` | Build unit tests |
| `WIN_UTILS_AUTOSTART_GTEST_SEARCH_MODE` | `Auto` | GTest detection: `Auto`, `Force`, `Skip` |

### Commands

```bash
# Configure with tests
cmake -B build -DWIN_UTILS_AUTOSTART_ENABLE_TESTS=ON

# Build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure -C Release
```

---

## License

MIT License — see [LICENSE](LICENSE) for details.

Copyright (c) 2026 Ihor Drachuk

---

## Author

**Ihor Drachuk** — [ihor-drachuk-libs@pm.me](mailto:ihor-drachuk-libs@pm.me)

[GitHub](https://github.com/ihor-drachuk/win-utils-autostart)
