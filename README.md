<div align=center>
  <img width=64 src="ui/assets/image/icon/evento.svg">
</div>

<h1 align="center">
  SAST Evento
</h1>
<p align="center">
  NJUPT SAST C++组 SoC项目
</p>

<p align="center">
    <img src="https://img.shields.io/badge/license-MIT-blue.svg">
    <img src="https://img.shields.io/badge/gui-slint-blueviolet">
    <img src="https://img.shields.io/badge/lang-C%2B%2B20-yellow.svg">
    <img src="https://img.shields.io/badge/platform-windows%20%7C%20macos%20%7C%20linux-lightgreen.svg">
</p>

## 关于

这个仓库包含了 C++ 组 2024 年暑期代码活动的任务和项目。该活动由 SAST 组织，旨在为学生提供学习和贡献开源项目的机会。

## 简介

SAST Evento 是一个 SAST 的事件管理系统，平时我们在活动过程中一般都会遇到下面的问题：
- 活动信息收集仍然通过传统的共享表格，效率低
- 活动时间安排冲突需要手动排查、活动场地冲突也要手动排查
- 活动计划表虽然模板一样，但是每周都需要人手动更新，没法自动生成
- 同学们活动没有的反馈不高，活动完的收获没法量化
- 活动的质量没有比较好的反馈

针对上面的问题，我们决定SoC期间制作一个活动辅助系统帮助部长和讲师们更加顺畅地完成日常活动，减少沟通负担。

本项目是SAST Evento的跨平台桌面客户端版本。

## License

项目基于 [MIT License](./LICENSE) 发布

## 参与开发

### 先决条件

- 一个支持 C++20 或更高标准的编译器
- CMake 3.15 或更高版本
- vcpkg 包管理器
- Rust 工具链

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

### 项目依赖
- [Boost.Beast](https://github.com/boostorg/beast)
- [Boost.Url](https://github.com/boostorg/url)
- [OpenSSL](https://github.com/openssl/openssl)
- [Slint](https://github.com/slint-ui/slint)
- [Google Test](https://github.com/google/googletest)
- [nlohmann-json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)
- [keychain](https://github.com/hrantzsch/keychain.git)