#include <Infrastructure/Cache/Cache.h>
#include <format>

namespace evento {

std::string CacheManager::generateKey(http::verb verb,
                                      urls::url_view url,
                                      const std::initializer_list<urls::param>& params) {
    std::string key = std::format("{}|{}", url.data(), static_cast<int>(verb));
    for (const auto& param : params) {
        key += std::format("|{}={}", param.key, param.value);
    }
    return key;
}

bool CacheManager::isExpired(const CacheEntry& entry) {
    using namespace std::chrono_literals;
    return std::chrono::steady_clock::now() - entry.insertTime >= 1h;
}

void CacheManager::insert(const std::string& key, const CacheEntry& entry) {
    auto it = _cacheMap.find(key);
    if (it != _cacheMap.end()) {
        _cacheList.erase(it->second);
        currentCacheSize -= it->second->second.size;
    }

    _cacheList.emplace_front(key, entry);
    _cacheMap[key] = _cacheList.begin();
    currentCacheSize += entry.size;

    while (currentCacheSize > MAX_CACHE_SIZE) {
        auto last = _cacheList.end();
        --last;
        currentCacheSize -= last->second.size;
        _cacheMap.erase(last->first);
        _cacheList.pop_back();
    }
}

std::optional<CacheEntry> CacheManager::get(std::string const& key) {
    if (isExpired(_cacheMap[key]->second)) {
        currentCacheSize -= _cacheMap[key]->second.size;
        _cacheList.erase(_cacheMap[key]);
        _cacheMap.erase(key);
        return std::nullopt;
    }
    auto it = _cacheMap.find(key);
    if (it == _cacheMap.end()) {
        return std::nullopt;
    }
    return it->second->second;
}

void CacheManager::load() {}

void CacheManager::save() {}

void CacheManager::saveToDisk() {}

std::string CacheManager::generateFilename(urls::url_view url) {
    return "";
}

} // namespace evento