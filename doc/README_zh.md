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

## :pushpin: 简介

SAST Evento 是一个 SAST 的事件管理系统，平时我们在活动过程中一般都会遇到下面的问题：

- 活动信息收集仍然通过传统的共享表格，效率低
- 活动时间安排冲突需要手动排查、活动场地冲突也要手动排查
- 活动计划表虽然模板一样，但是每周都需要人手动更新，没法自动生成
- 同学们活动没有的反馈不高，活动完的收获没法量化
- 活动的质量没有比较好的反馈

针对上面的问题，我们决定 SoC 期间制作一个活动辅助系统帮助部长和讲师们更加顺畅地完成日常活动，减少沟通负担。

这个项目是 SAST Evento 的桌面客户端版本。

## :mag: 预览

<div align=center>
  <img src="img/preview.png">
</div>

## :cyclone: 平台支持

| 平台                 | 状态 | 说明                |
| -------------------- | ---- | ------------------- |
| Windows x64          | ✅    |                     |
| Windows arm64        | :x:  | 等待 Slint 上游修复 |
| macOS arm64          | ✅    |                     |
| macOS x64            | :x:  | 欢迎提交 PR         |
| Linux x64 (pacman)   | ✅    |                     |
| Linux x64 (portage)  | ✅    |                     |
| Linux x64 (deb)      | ✅    |                     |
| Linux x64 (rpm)      | :x:  | 欢迎提交 PR         |
| Linux x64 (nix)      | :x:  | 欢迎提交 PR         |
| Linux x64 (AppImage) | :x:  | 欢迎提交 PR         |

## 📦 安装

通常，您可以从[发布页面](https://github.com/NJUPT-SAST/sast-evento/releases)下载最新版本。如果您使用以下 Linux 发行版之一，可以通过包管理器直接安装该软件包。

### Arch Linux

```bash
paru -S sast-evento
# 或
yay -S sast-evento
```
> [!NOTE]  
> 如果您想安装预构建的二进制包，可以安装 `sast-evento-bin` 包；如果您想使用 `dev` 分支，可以安装 `sast-evento-git` 包。

### Gentoo Linux

添加 `corcodile` overlay。
```bash
emerge -av eselect-repository
eselect repository add corcodile git https://github.com/f3rmata/corcodile.git
```

使用 overlay 中的 `spdlog` 解决捆绑的 `fmt` 问题。
```bash
emerge -av spdlog::corcodile sast-evento
```

## :open_file_folder: 生成的文件

### 配置

在通常情况下，您不需要直接修改此文件。配置文件位于以下路径：

- Windows: `%AppData%\Local\NJUPT-SAST-C++\SAST-Evento\config.toml`
- macOS 和 Linux: `$HOME/.config/NJUPT-SAST-C++/SAST-Evento/config.toml`

### 日志

日志文件位于 log 文件夹中。报告错误时请提交日志文件。log 文件夹位于以下路径：

- Windows: `%Temp%\NJUPT-SAST\logs`
- macOS: `$TEMPDIR/NJUPT-SAST/logs`
- Linux: `/tmp/NJUPT-SAST/logs`

### 缓存

缓存文件夹位于以下路径：

- Windows: `%LocalAppData%\evento`
- macOS: `$HOME/Library/Caches/evento`
- Linux: `$HOME/.cache/evento`

## :triangular_ruler: 开发

### 先决条件

- 支持 C++20 或更高标准的编译器
- CMake 3.21 或更高版本
- vcpkg 包管理器
- Rust 工具链

对于 Linux 平台，我们建议您直接从包管理器安装 Qt6 基础库：

```bash
# 对于 Arch Linux
sudo pacman -S qt6-base
# 对于 Ubuntu
sudo apt install qt6-base-dev
```

对于 macOS 和 Windows 平台，您可以从官方网站安装 Qt6 以动态链接 Qt 到此项目；或者，您可以使用 vcpkg 从源代码构建 Qt6。

### 克隆

```bash
git clone --recursive https://github.com/NJUPT-SAST/sast-evento.git
```

注意：此项目使用子模块，因此请确保使用 `--recursive` 标志克隆存储库，或者在克隆后执行以下命令：

```bash
git submodule update --init --recursive
```

 ### 提交检查 Hook

此项目使用 [pre-commit](https://pre-commit.com/) 进行提交检查，以确保代码风格一致性。请先安装 pre-commit 工具：

```bash
# 对于 Arch Linux
sudo pacman -S pre-commit
# 对于 Pipx 用户（跨平台）
pipx install pre-commit
```

然后，在克隆项目后，执行以下命令安装 pre-commit 钩子：

```bash
pre-commit install
```

> [!TIP]  
> 如果您发现工具提供的结果不可靠，可以使用 `git commit --no-verify` 暂时跳过提交检查。

### 构建

> [!TIP]  
> 我们建议使用 VScode 打开和编辑项目。我们已经保留了 `.vscode` 文件夹用于基本设置和扩展。

此项目使用 CMake Presets 进行快速配置和构建。所需的命令行如下：

```bash
# 对于 Windows 平台，请确保配置了编译工具集相关的环境变量
# 您可以使用 `vcpkg env` 命令进入一个设置了正确环境变量的 shell
cmake --preset native
# 根据需要，您可以使用 `native-debug`、`native-release` 或 `native-relwithdebinfo` 预设
cmake --build --preset native
```

如果您使用 vcpkg 安装 Qt6，需要在 CMake 命令中添加以下构建选项：

```bash
cmake --preset native -DVCPKG_MANIFEST_FEATURES=qt-from-vcpkg
```

  对于 Windows 平台，您可以使用静态链接以避免一些奇怪的问题：

```bash
cmake --preset native -DVCPKG_MANIFEST_FEATURES=qt-from-vcpkg -DVCPKG_TARGET_TRIPLET=<x64 或 arm64>-windows-static
```

如果您想加快 **Debug** 模式下的构建过程，可以添加 `-DSPEED_UP_DEBUG_BUILD=ON` 选项。

## :rainbow: 贡献

欢迎提交拉取请求和任何反馈。对于重大更改，请先打开一个 issue 讨论您想要更改的内容。

### 感谢所有贡献者

[![Contributors](https://contrib.rocks/image?repo=NJUPT-SAST/sast-evento)]("https://github.com/NJUPT-SAST/sast-evento/graphs/contributors")

## :link: 链接

### 相关项目

[![Readme Card](https://github-readme-stats.vercel.app/api/pin/?username=NJUPT-SAST&repo=sast-evento-backend)](https://github.com/NJUPT-SAST/sast-evento-backend)
[![Readme Card](https://github-readme-stats.vercel.app/api/pin/?username=NJUPT-SAST&repo=sast-evento-mobile)](https://github.com/NJUPT-SAST/sast-evento-mobile)
[![Readme Card](https://github-readme-stats.vercel.app/api/pin/?username=NJUPT-SAST&repo=sast-link-backend)](https://github.com/NJUPT-SAST/sast-evento-link)
[![Readme Card](https://github-readme-stats.vercel.app/api/pin/?username=NJUPT-SAST&repo=sast-link)](https://github.com/NJUPT-SAST/sast-link)

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