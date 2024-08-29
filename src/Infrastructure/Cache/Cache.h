#pragma once

#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <boost/url/url_view.hpp>
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
    nlohmann::basic_json<> result;
    std::chrono::steady_clock::time_point insertTime;
    std::size_t size; //cache size
};

class CacheManager {
public:
    CacheManager() { load(); }
    ~CacheManager() { save(); }

    static std::string generateKey(http::verb verb,
                                   urls::url_view url,
                                   const std::initializer_list<urls::param>& params);

    static std::string generateFilename(urls::url_view url);

    static bool isExpired(const CacheEntry& entry);

    void insert(const std::string& key, const CacheEntry& entry);

    std::optional<CacheEntry> get(std::string const& key);

    static constexpr size_t MAX_CACHE_SIZE = 64 * 1024 * 1024;

private:
    void load();
    void save();

    void saveToDisk();

    std::list<std::pair<std::string, CacheEntry>> _cacheList;
    std::unordered_map<std::string, decltype(_cacheList.begin())> _cacheMap;
    inline static size_t currentCacheSize = 0;
};

} // namespace evento