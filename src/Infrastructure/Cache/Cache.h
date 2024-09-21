#pragma once

#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <chrono>
#include <list>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>

namespace evento {

namespace beast = boost::beast;
namespace http = beast::http;
namespace urls = boost::urls;

struct CacheEntry {
    nlohmann::basic_json<> data;
    std::chrono::steady_clock::time_point insertTime;
    std::chrono::steady_clock::duration ttl;
    std::size_t size; //cache size
};

class CacheManager {
public:
    static std::string generateKey(http::verb verb,
                                   urls::url_view url,
                                   const std::initializer_list<urls::param>& params);

    static std::string generateStem(urls::url_view url);

    static std::optional<std::filesystem::path> cacheDir();

    static bool isExpired(const CacheEntry& entry);

    static bool saveToDisk(std::string const& data, std::filesystem::path const& path);

    void insert(const std::string& key, const CacheEntry& entry);

    static std::size_t currentCacheSize() { return _currentCacheSize; }

    std::optional<CacheEntry> get(std::string const& key);

    void clear();

    static constexpr size_t MAX_CACHE_SIZE = 64 * 1024 * 1024;

private:
    std::list<std::pair<std::string, CacheEntry>> _cacheList{};
    std::unordered_map<std::string, std::list<std::pair<std::string, CacheEntry>>::iterator>
        _cacheMap{};
    inline static std::size_t _currentCacheSize = 0;
};

} // namespace evento