#pragma once

#include <boost/url/url_view.hpp>
#include <boost/url/urls.hpp>
#include <spdlog/spdlog.h>

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

} // namespace evento