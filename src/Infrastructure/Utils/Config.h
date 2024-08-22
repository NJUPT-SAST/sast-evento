#pragma once

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <toml++/toml.h>

/*
[account.<user-id>]
expire = <date-time>

[setting]
language = <int>
minimal-to-tray = <bool>
notice-begin = <bool>
notice-end = <bool>
theme = <int>
*/

namespace evento {

namespace details {

const std::filesystem::path configDir =
#if defined(_WIN32) || defined(_WIN64)
    std::filesystem::path(std::getenv("APPDATA")) / "Local" / "NJUPT-SAST-C++" / "SAST-Evento";
#else
    std::filesystem::path(std::getenv("HOME")) / ".config" / "NJUPT-SAST-C++" / "SAST-Evento";
#endif

} // namespace details

inline toml::table config;

inline struct Setting {
    int language;
    bool minimalToTray;
    bool noticeBegin;
    bool noticeEnd;
    int theme;
} settings;

static void loadSetting() {
    if (!config.contains("setting")) {
        config.insert("setting", toml::table{});
    }
    auto& setting = config["setting"].ref<toml::table>();

    auto languageIdx = setting["language"].value_or(0);
    if (languageIdx > 2) {
        languageIdx = 0;
    }
    auto themeIdx = setting["theme"].value_or(0);
    if (themeIdx > 2) {
        themeIdx = 0;
    }
    auto noticeBegin = setting["notice-begin"].value_or(false);
    auto noticeEnd = setting["notice-end"].value_or(false);
    auto minimalToTray = setting["minimal-to-tray"].value_or(false);

    evento::settings = {
        .language = languageIdx,
        .minimalToTray = minimalToTray,
        .noticeBegin = noticeBegin,
        .noticeEnd = noticeEnd,
        .theme = themeIdx,
    };
}

static void loadAccount() {
    if (!config.contains("account")) {
        config.insert("account", toml::table{});
    }
}

inline void initConfig() {
    if (!std::filesystem::exists(details::configDir)) {
        spdlog::info("Config directory does not exist, creating...");
        std::filesystem::create_directories(details::configDir);
    }
    auto path = details::configDir / "config.toml";
    if (!std::filesystem::exists(path)) {
        std::ofstream ofs(path);
        spdlog::info("Config file does not exist, creating...");
    }
    try {
        config = toml::parse_file(path.u8string());
    } catch (const toml::parse_error& err) {
        spdlog::error("\"{}\" could not be opened for parsing.", path.string());
        config = toml::parse("");
    }

    loadSetting();
    loadAccount();
}

inline void saveConfig() {
    auto path = details::configDir / "config.toml";
    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        spdlog::error("Failed to open \"{}\" for saving config.", path.string());
        return;
    }
    ofs << config;
}

} // namespace evento