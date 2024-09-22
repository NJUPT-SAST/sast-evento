<div align=center>
  <img width=64 src="ui/assets/image/icon/evento.svg">
</div>

<h1 align="center">
  SAST Evento
</h1>
<p align="center">
A cross-platform desktop client based on Slint
</p>

<p align="center">
    <img src="https://img.shields.io/badge/language-C%2B%2B20-yellow.svg">
    <img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgreen.svg">
    <img src="https://img.shields.io/badge/license-MIT-blue.svg">
</p>

<p align="center">
  <a href="https://slint.dev">
      <img alt="#MadeWithSlint" src="https://raw.githubusercontent.com/slint-ui/slint/master/logo//MadeWithSlint-logo-light.svg" height="60">
  </a>
</p>

<p align="center">
    English | <a href="./doc/README_zh.md">简体中文</a>
</p>

## Brief

SAST Evento is an event management system for SAST. During our activities, we often encounter the following problems:
- The collection of event information is still done through traditional shared spreadsheets, which is inefficient.
- Manual investigation is required to resolve conflicts in event scheduling and venue availability.
- The event schedule template needs to be manually updated every week and cannot be generated automatically.
- The participation and feedback from students are not high, and the achievements of the events cannot be quantified.
- There is a lack of quality feedback for the events.

To address these issues, we have decided to develop an event assistance system during the SoC period to help department heads and instructors smoothly manage their daily activities and reduce communication burden.

This project is the desktop client version of SAST Evento.

## Platform Support

| Platform           | Status                   | Instruction                     |
| ------------------ | ------------------------ | ------------------------------- |
| Windows x64        | ✅                        |
| Windows arm64      | :heavy_exclamation_mark: | Application cannot run normally |
| macOS arm64        | ✅                        |
| macOS x64          | :x:                      | No packaging plan available     |
| Linux x64 (pacman) | ✅                        |
| Linux x64 (deb)    | ✅                        |
| Linux x64 (rpm)    | :x:                      | No packaging plan available     |
| Linux x64 (nix)    | :x:                      | No packaging plan available     |
| Linux arm64        | :x:                      | No packaging plan available     |

## Development

### Prerequisites

- Compiler that supports C++20 or higher standard
- CMake 3.21 or higher version
- vcpkg package manager
- Rust toolchain

For Linux platforms, you also need to install the Qt6 base library:

```bash
# For Arch Linux
sudo pacman -S qt6-base
# For Ubuntu
sudo apt install qt6-base-dev
```

For macOS and Windows platforms, you can install Qt6 from the official website; alternatively, you can use vcpkg to build Qt6 from source to ensure static linking to the executable, which requires enabling the `-DVCPKG_MANIFEST_FEATURES=qt-from-vcpkg` build option.

### Clone

```bash
git clone --recursive https://github.com/NJUPT-SAST/sast-evento.git
```

Note: This project uses submodules, so make sure to clone the repository with the `--recursive` flag, or execute the following command after cloning:

```bash
git submodule update --init --recursive
```

### Commit Check Hook
This project uses [pre-commit](https://pre-commit.com/) for commit checks to ensure code style consistency. Please install the pre-commit tool first:
```bash
# For Arch Linux
sudo pacman -S pre-commit
# For Pipx users (cross-platform)
pipx install pre-commit
```

Then, after cloning the project, execute the following command to install the pre-commit hook:

```bash
pre-commit install
```

> [!TIP]  
> If you find the results provided by the tool unreliable, you can temporarily skip the commit check by using `git commit --no-verify`.

### Build
> [!TIP]  
> We recommend using VScode to open and edit the project. We have reserved the `.vscode` folder for basic settings and extensions.

This project uses CMake Presets for quick configuration and building. The required command line is as follows:

```bash
# For Windows platform, make sure the compilation toolset-related environment variables are configured
# You can use the `vcpkg env` command to enter a shell with the correct environment variables set
cmake --preset native
# Depending on your needs, you can use `native-debug`, `native-release`, or `native-relwithdebinfo` preset
cmake --build --preset native
```

### Project Dependencies
- [Boost.Beast](https://github.com/boostorg/beast)
- [Boost.Url](https://github.com/boostorg/url)
- [Boost.Process](https://github.com/boostorg/process)
- [OpenSSL](https://github.com/openssl/openssl)
- [Slint](https://github.com/slint-ui/slint)
- [toml++](https://github.com/marzer/tomlplusplus)
- [nlohmann-json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)
- [keychain](https://github.com/hrantzsch/keychain.git)