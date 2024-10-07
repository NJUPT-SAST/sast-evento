#include <Infrastructure/Cache/Cache.h>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>

namespace evento {

namespace fs = std::filesystem;

std::string CacheManager::generateKey(http::verb verb,
                                      urls::url_view url,
                                      const std::initializer_list<urls::param>& params) {
    std::string key = std::format("{}|{}", url.data(), static_cast<int>(verb));
    for (const auto& param : params) {
        key += std::format("|{}={}", param.key, param.value);
    }
    return key;
}

std::string CacheManager::generateStem(urls::url_view url) {
    return std::to_string(std::hash<std::string>{}(url.data()));
}

bool CacheManager::isExpired(const CacheEntry& entry) {
    return std::chrono::steady_clock::now() - entry.insertTime >= entry.ttl;
}

std::optional<std::filesystem::path> CacheManager::cacheDir() {
    fs::path cacheFileDir;
#ifdef PLATFORM_WINDOWS
    auto localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData == nullptr) {
        spdlog::warn("LOCALAPPDATA environment variable not found");
        return std::nullopt;
    }
    cacheFileDir = fs::path(localAppData) / "Programs" / "evento";

#elif defined(PLATFORM_LINUX)
    auto xdgCacheHome = std::getenv("XDG_CACHE_HOME");
    if (xdgCacheHome == nullptr) {
        auto home = std::getenv("HOME");
        if (home == nullptr) {
            spdlog::warn("HOME environment variable not found");
            return std::nullopt;
        }
        cacheFileDir = fs::path(home) / ".cache" / "evento";
    } else {
        cacheFileDir = fs::path(xdgCacheHome) / "evento";
    }
#elif defined(PLATFORM_APPLE)
    auto home = std::getenv("HOME");
    if (home == nullptr) {
        spdlog::warn("HOME environment variable not found");
        return std::nullopt;
    }
    cacheFileDir = fs::path(home) / "Library" / "Caches" / "evento";
#else
#error "Unsupported platform"
#endif

    if (!fs::exists(cacheFileDir) || !fs::is_directory(cacheFileDir)) {
        fs::create_directory(cacheFileDir);
    }

    return fs::absolute(cacheFileDir);
}

void CacheManager::insert(const std::string& key, const CacheEntry& entry) {
    auto it = _cacheMap.find(key);
    if (it != _cacheMap.end()) {
        _cacheList.erase(it->second);
        _currentCacheSize -= it->second->second.size;
    }

    _cacheList.emplace_front(key, entry);
    _cacheMap[key] = _cacheList.begin();
    _currentCacheSize += entry.size;

    while (_currentCacheSize > MAX_CACHE_SIZE) {
        auto last = _cacheList.end();
        --last;
        _currentCacheSize -= last->second.size;
        _cacheMap.erase(last->first);
        _cacheList.pop_back();
    }
}

std::optional<CacheEntry> CacheManager::get(std::string const& key) {
    if (_cacheMap.empty()) {
        return std::nullopt;
    }

    auto it = _cacheMap.find(key);

    if (it == _cacheMap.end()) {
        return std::nullopt;
    }

    if (isExpired(it->second->second)) {
        _currentCacheSize -= _cacheMap[key]->second.size;
        _cacheList.erase(_cacheMap[key]);
        _cacheMap.erase(key);
        return std::nullopt;
    }

    return it->second->second;
}

void CacheManager::clear() {
    _cacheList.clear();
    _cacheMap.clear();
    _currentCacheSize = 0;
    if (auto dir = cacheDir()) {
        fs::remove_all(*dir);
    }
}

void CacheManager::clearMemoryCache() {
    _cacheList.clear();
    _cacheMap.clear();
    _currentCacheSize = 0;
}

} // namespace evento
