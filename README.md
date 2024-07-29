<div align=center>
  <img width=64 src="ui/assets/app_icon.svg">
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

## 开始

### 先决条件

- 一个支持C++20或更高标准的编译器
- CMake 3.15 或更高版本
- vcpkg包管理器
- rust工具链

### 克隆与构建

- 克隆此仓库

```
git clone --recursive https://github.com/NJUPT-SAST/sast-evento.git
``````

我们建议使用VScode打开和编辑项目。我们特别保留`.vscode`文件夹用于基本设置和扩展。

- 构建

Windows Cmd:
```shell
cmake -B build -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```
Windows Powershell:
```shell
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$Env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```
Linux & MacOS:
```shell
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

### 项目依赖
- [Boost.Beast](https://github.com/boostorg/beast)
- [Boost.Url](https://github.com/boostorg/url)
- [OpenSSL](https://github.com/openssl/openssl)
- [Slint](https://github.com/slint-ui/slint)
- [Google Test](https://github.com/google/googletest)
- [nlohmann-json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)
