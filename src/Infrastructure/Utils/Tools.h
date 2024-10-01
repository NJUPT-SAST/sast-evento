#pragma once

#include <Infrastructure/Network/ResponseStruct.h>
#include <array>
#include <boost/url/url_view.hpp>
#include <boost/url/urls.hpp>
#include <ctime>
#include <spdlog/spdlog.h>

namespace evento {

namespace urls = boost::urls;

inline void openBrowser(urls::url_view url) {
    constexpr auto commandFmtStr =
#ifdef PLATFORM_LINUX
        R"(xdg-open "{}")";
#elif defined(PLATFORM_WINDOWS)
        R"(start "" "{}")";
#elif defined(PLATFORM_APPLE)
        R"(open "{}")";
#else
#error "Unsupported platform"
#endif
    auto command = std::format(commandFmtStr, url.data());
    std::thread{[commandFmtStr, command] {
        if (auto result = std::system(command.c_str())) {
            spdlog::error("Failed to open browser, error code: {}", result);
        }
    }}.detach();
}

inline time_t parseIso8601Utc(const char* date) {
    struct tm tt = {0};
    double seconds;
    if (sscanf(date,
#ifdef EVENTO_API_V1
               "%04d-%02d-%02d %02d:%02d:%lf",
#else
               "%04d-%02d-%02dT%02d:%02d:%lfZ",
#endif
               &tt.tm_year,
               &tt.tm_mon,
               &tt.tm_mday,
               &tt.tm_hour,
               &tt.tm_min,
               &seconds)
        != 6)
        return -1;
    tt.tm_sec = (int) seconds;
    tt.tm_mon -= 1;
    tt.tm_year -= 1900;
    tt.tm_isdst = -1;
#ifdef _MSC_VER
    return _mkgmtime(&tt);
#else
    return timegm(&tt);
#endif
}

inline std::string firstDateTimeOfWeek() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto tm = std::localtime(&time);

    // Find the start of the week (Monday)
    int daysSinceMonday = (tm->tm_wday + 6) % 7;
    auto startOfWeek = now - std::chrono::hours(24 * daysSinceMonday);

    // Format the date
    std::stringstream ss;
    auto startOfWeekTime = std::chrono::system_clock::to_time_t(startOfWeek);
    ss << std::put_time(std::gmtime(&startOfWeekTime), "%Y-%m-%dT%H:%M:%S.000Z");
    return ss.str();
}

inline std::string guessImageExtByBytes(std::array<unsigned char, 4> bytes) {
    if (bytes[0] == 0xff && bytes[1] == 0xd8 && bytes[2] == 0xff) {
        return "jpg";
    }
    if (bytes[0] == 0x89 && bytes[1] == 0x50 && bytes[2] == 0x4e && bytes[3] == 0x47) {
        return "png";
    }
    if (bytes[0] == 0x47 && bytes[1] == 0x49 && bytes[2] == 0x46) {
        return "gif";
    }
    if (bytes[0] == 0x42 && bytes[1] == 0x4d) {
        return "bmp";
    }
    return "unknown";
}

} // namespace evento