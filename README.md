<div align="center">
  <img
    src="https://raw.githubusercontent.com/LizardByte/.github/refs/heads/master/branding/logos/logo.svg"
    alt="LizardByte icon"
    width="256"
  />
  <h1 align="center">libdisplaydevice</h1>
  <h4 align="center">C++ library to modify display devices.</h4>
</div>

<div align="center">
  <a href="https://github.com/LizardByte/libdisplaydevice"><img src="https://img.shields.io/github/stars/lizardbyte/libdisplaydevice.svg?logo=github&style=for-the-badge" alt="GitHub stars"></a>
  <a href="https://github.com/LizardByte/libdisplaydevice/actions/workflows/ci.yml?query=branch%3Amaster"><img src="https://img.shields.io/github/actions/workflow/status/lizardbyte/libdisplaydevice/ci.yml.svg?branch=master&label=CI%20build&logo=github&style=for-the-badge" alt="GitHub Workflow Status (CI)"></a>
  <a href="https://codecov.io/gh/LizardByte/libdisplaydevice"><img src="https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fapp.lizardbyte.dev%2Fdashboard%2Fshields%2Fcodecov%2Flibdisplaydevice.json&style=for-the-badge&logo=codecov" alt="Codecov"></a>
  <a href="https://sonarcloud.io/project/overview?id=LizardByte_libdisplaydevice"><img src="https://img.shields.io/sonar/quality_gate/LizardByte_libdisplaydevice.svg?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarqubecloud&label=sonarcloud" alt="SonarCloud"></a>
</div>

# Overview

## ℹ️ About

LizardByte has the full documentation hosted on [Read the Docs](https://libdisplaydevice.readthedocs.io/).

libdisplaydevice is a library that provides a common interface for interacting with display devices.
It is intended to be used by applications that need to interact with displays, such as screen capture software,
remote desktop software, and video players.

Windows and macOS are currently supported. Support for Linux is planned for the future.

## 🛠️ Build

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

## ❓ Support

Our support methods are listed in our [LizardByte Docs](https://lizardbyte.readthedocs.io/latest/about/support.html).

<details style="display: none;">
  <summary></summary>
  [TOC]
</details>
