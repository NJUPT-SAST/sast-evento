#pragma once

#include <string>

struct SystemInfo {
    std::string desktopEnvironment; // DE: Fluent
    std::string windowManager;      // WM: Desktop Window Manager
    std::string wmTheme;            // WM Theme: Oem - Blue (System: Light, Apps: Light)
    std::string icons;              // Icons: Recycle Bin
    std::string font;               // Font: Microsoft YaHei UI (12pt)
    std::string cursor;             // Cursor: Windows Default (32px)
};

/**
 * @brief Retrieves the information about the window manager.
 * @return The `SystemInfo` struct containing the window manager information.
 */
[[nodiscard]] auto getSystemInfo() -> SystemInfo;
