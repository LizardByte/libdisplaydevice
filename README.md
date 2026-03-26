# Overview

[![GitHub Workflow Status (CI)](https://img.shields.io/github/actions/workflow/status/lizardbyte/libdisplaydevice/ci.yml.svg?branch=master&label=CI%20build&logo=github&style=for-the-badge)](https://github.com/LizardByte/libdisplaydevice/actions/workflows/ci.yml?query=branch%3Amaster)
[![Codecov](https://img.shields.io/codecov/c/gh/LizardByte/libdisplaydevice?token=goyvmDl6J5&style=for-the-badge&logo=codecov&label=codecov)](https://codecov.io/gh/LizardByte/libdisplaydevice)
[![GitHub stars](https://img.shields.io/github/stars/lizardbyte/libdisplaydevice.svg?logo=github&style=for-the-badge)](https://github.com/LizardByte/libdisplaydevice)

## About

LizardByte has the full documentation hosted on [Read the Docs](https://libdisplaydevice.readthedocs.io/).

libdisplaydevice is a WIP library that provides a common interface for interacting with display devices.
It is intended to be used by applications that need to interact with displays, such as screen capture software,
remote desktop software, and video players.

Initial support is planned for Windows, but could be expanded to other platforms in the future.

## Build

### Clone

Ensure [git](https://git-scm.com/) is installed and run the following:

```bash
git clone https://github.com/lizardbyte/libdisplaydevice.git --recurse-submodules
cd libdisplaydevice
mkdir -p build
```

### Windows

First, you need to install [MSYS2](https://www.msys2.org).

For AMD64 startup "MSYS2 UCRT64" (or for ARM64 startup "MSYS2 CLANGARM64") then execute the following commands.

#### Update all packages
```bash
pacman -Syu
```

#### Set toolchain variable
For UCRT64:
```bash
export TOOLCHAIN="ucrt-x86_64"
```

For CLANGARM64:
```bash
export TOOLCHAIN="clang-aarch64"
```

#### Install dependencies
```bash
dependencies=(
  "doxygen"
  "mingw-w64-${TOOLCHAIN}-boost"
  "mingw-w64-${TOOLCHAIN}-cmake"
  "mingw-w64-${TOOLCHAIN}-graphviz"
  "mingw-w64-${TOOLCHAIN}-ninja"
  "mingw-w64-${TOOLCHAIN}-nlohmann-json"
  "mingw-w64-${TOOLCHAIN}-nodejs"
  "mingw-w64-${TOOLCHAIN}-toolchain"
)
pacman -S "${dependencies[@]}"
```

### Build

```bash
cmake -G Ninja -B build -S .
ninja -C build
```

### Test

```bash
./build/tests/test_libdisplaydevice
```

## Support

Our support methods are listed in our [LizardByte Docs](https://lizardbyte.readthedocs.io/latest/about/support.html).

<details style="display: none;">
  <summary></summary>
  [TOC]
</details>
