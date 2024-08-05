#pragma once

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <toml++/toml.h>

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