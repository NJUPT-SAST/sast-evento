#pragma once

#include <boost/url/url_view.hpp>
#include <boost/url/urls.hpp>
#include <spdlog/spdlog.h>
#include <ctime>

namespace evento {

namespace urls = boost::urls;

inline void openBrowser(urls::url_view url) {
    using namespace std::string_literals;
    std::string url_str = "\""s + url.data() + "\"";
#ifdef PLATFORM_LINUX
    auto command = "xdg-open " + url_str;
#elif defined(PLATFORM_WINDOWS)
    auto command = "start \"\" " + url_str;
#elif defined(PLATFORM_APPLE)
    auto command = "open " + url_str;
#else
#error "Unsupported platform"
#endif

    std::thread t([command]() {
        if (std::system(command.c_str()) != 0) {
            spdlog::info("open browser failed");
        }
    });
    t.detach();
}

inline time_t parseIso8601Utc(const char* date) {
    struct tm tt = {0};
    double seconds;
    if (sscanf(date,
               "%04d-%02d-%02dT%02d:%02d:%lfZ",
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

} // namespace evento