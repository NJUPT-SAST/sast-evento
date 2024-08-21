#pragma once

#include <boost/url/url_view.hpp>
#include <boost/url/urls.hpp>

namespace evento {

namespace urls = boost::urls;

inline void openBrowser(urls::url_view url) {
    using namespace std::string_literals;
    std::string url_str = "\""s + url.data() + "\"";
#ifdef __linux__
    system(("xdg-open "s + url_str).c_str());
#elif defined(_WIN32) || defined(_WIN64)
    system(("start \"\" "s + url_str).c_str());
#elif defined(__APPLE__)
    system(("open "s + url_str).c_str());
#else
    spdlog::error("unsupported os");
#endif
}

} // namespace evento