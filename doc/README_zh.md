<div align=center>
  <img width=64 src="../ui/assets/image/icon/evento.svg">
</div>

<h1 align="center">
  SAST Evento
</h1>
<p align="center">
基于 Slint 的跨平台桌面客户端
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
    <a href="../README.md">English</a> | 简体中文
</p>

## 简介

SAST Evento 是一个 SAST 的事件管理系统，平时我们在活动过程中一般都会遇到下面的问题：

- 活动信息收集仍然通过传统的共享表格，效率低
- 活动时间安排冲突需要手动排查、活动场地冲突也要手动排查
- 活动计划表虽然模板一样，但是每周都需要人手动更新，没法自动生成
- 同学们活动没有的反馈不高，活动完的收获没法量化
- 活动的质量没有比较好的反馈

针对上面的问题，我们决定 SoC 期间制作一个活动辅助系统帮助部长和讲师们更加顺畅地完成日常活动，减少沟通负担。

本项目是 SAST Evento 的桌面客户端版本。

## 预览

<div align=center>
  <img src="../doc/img/preview.png">
</div>

## 项目平台支持

| 平台               | 状态 | 说明         |
| ------------------ | ---- | ------------ |
| Windows x64        | ✅   |              |
| Windows arm64      | :x:  | 暂无打包计划 |
| macOS arm64        | ✅   |              |
| macOS x64          | :x:  | 暂无打包计划 |
| Linux x64 (pacman) | ✅   |              |
| Linux x64 (portage)| ✅   |              |
| Linux x64 (deb)    | ✅   |              |
| Linux x64 (rpm)    | :x:  | 暂无打包计划 |
| Linux x64 (nix)    | :x:  | 暂无打包计划 |
| Linux arm64        | :x:  | 暂无打包计划 |

## 安装

对于 Linux 用户，您可以使用下面的命令安装：

Arch Linux

```bash
paru -S sast-evento-bin
# or
yay -S sast-evento-bin
```
> [!TIP]  
> 如果您想安装预编译的二进制包，请使用 `sast-evento-bin` 包，或者如果您想使用 `dev` 分支，请使用 `sast-evento-git` 包。

如果不想使用 AUR，或者使用其他系统，您可以在 [GitHub Release](https://github.com/NJUPT-SAST/sast-evento/releases) 页面下载最新的安装包。

Gentoo Linux

```bash
# Add corcodile overlay.
emerge -av eselect-repository
eselect repository add corcodile git https://github.com/f3rmata/corcodile.git

# Using spdlog in overlay to fix the bundled fmt issue.
emerge -av spdlog::corcodile sast-evento
```

## 参与开发

### 先决条件

- 支持 C++20 或更高标准的编译器
- CMake 3.21 或更高版本
- vcpkg 包管理器
- Rust 工具链

对于 Linux 平台，我们建议您直接从包管理器安装 Qt6 基础库：

```bash
# For Arch Linux
sudo pacman -S qt6-base
# For Ubuntu
sudo apt install qt6-base-dev
```

对于 macOS 和 Windows 平台，您可以从官方网站安装 Qt6 以动态链接 Qt 到此项目；或者，您可以使用 vcpkg 从源代码构建 Qt6。

### 克隆

```bash
git clone --recursive https://github.com/NJUPT-SAST/sast-evento.git
```

注意：本项目使用了子模块，所以请确保使用 `--recursive` 参数来克隆仓库，或者在克隆后执行以下命令：

```bash
git submodule update --init --recursive
```

### 提交检查 Hook

本项目使用 [pre-commit](https://pre-commit.com/) 来进行提交检查，以确保代码风格一致性。请先安装 pre-commit 工具：

```bash
# For Arch Linux
sudo pacman -S pre-commit
# For Pipx users (cross-platform)
pipx install pre-commit
```

然后在克隆项目后执行以下命令安装 pre-commit 钩子：

```bash
pre-commit install
```

> [!TIP]  
> 如果您认为工具提供的结果不可靠，可使用 `git commit --no-verify` 临时跳过提交检查。

### 构建

> [!TIP]  
> 我们建议使用 VScode 打开和编辑项目。我们特别保留 `.vscode` 文件夹用于基本设置和扩展。

本项目使用 CMake Presets 来快速配置和构建项目，所需命令行如下：

```bash
# 对于 Windows 平台，请保证编译工具集相关环境变量已经配置
# 您可以使用 `vcpkg env` 命令进入正确设置环境变量的 shell
cmake --preset native
# 根据需要，可换用 `native-debug`, `native-release` 或 `native-relwithdebinfo` 预设
cmake --build --preset native
```

如果您使用 vcpkg 安装 Qt6，您需要在 CMake 命令中添加以下构建选项：

```bash
cmake --preset native -DVCPKG_MANIFEST_FEATURES=qt-from-vcpkg
```

对于 Windows 平台，您可以使用静态链接来避免一些奇怪的问题：

```bash
cmake --preset native -DVCPKG_MANIFEST_FEATURES=qt-from-vcpkg -DVCPKG_TARGET_TRIPLET=<x64 or arm64>-windows-static
```

如果您想加快 **Debug** 模式下的构建过程，可以添加 `-DSPEED_UP_DEBUG_BUILD=ON` 选项。

### 项目依赖

- [Boost.Beast](https://github.com/boostorg/beast)
- [Boost.Url](https://github.com/boostorg/url)
- [Boost.Process](https://github.com/boostorg/process)
- [OpenSSL](https://github.com/openssl/openssl)
- [Slint](https://github.com/slint-ui/slint)
- [toml++](https://github.com/marzer/tomlplusplus)
- [nlohmann-json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)
- [keychain](https://github.com/hrantzsch/keychain.git)
