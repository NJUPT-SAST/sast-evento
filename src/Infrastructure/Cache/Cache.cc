#include <Infrastructure/Cache/Cache.h>
#include <filesystem>
#include <format>
#include <fstream>
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

std::string CacheManager::generateFilename(urls::url_view url) {
    auto name = std::to_string(std::hash<std::string>{}(url.data()));
    auto extension = url.path().substr(url.path().find_last_of('.'));
    return name + extension;
}

bool CacheManager::isExpired(const CacheEntry& entry) {
    using namespace std::chrono_literals;
    return std::chrono::steady_clock::now() - entry.insertTime >= 1h;
}

std::optional<std::filesystem::path> CacheManager::cacheDir() {
    fs::path cacheFileDir;
#if defined(_WIN32) || defined(_WIN64)
    auto localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData == nullptr) {
        spdlog::warn("LOCALAPPDATA environment variable not found");
        return std::nullopt;
    }
    cacheFileDir = fs::path(localAppData) / "Programs" / "evento";

#elif __linux__
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
#elif __APPLE__
    auto home = std::getenv("HOME");
    if (home == nullptr) {
        spdlog::warn("HOME environment variable not found");
        return;
    }
    cacheFileDir = fs::path(home) / "Library" / "Caches" / "evento";
#else
#error "Unsupported platform"
#endif

    if (!fs::is_directory(cacheFileDir))
        return std::nullopt;

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
    if (isExpired(_cacheMap[key]->second)) {
        _currentCacheSize -= _cacheMap[key]->second.size;
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

bool CacheManager::saveToDisk(std::string const& data, fs::path const& path) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    if (file.is_open()) {
        file << data;
        return true;
    }
    spdlog::warn("Failed to save cache to disk: {}", path.string());
    return false;
}

} // namespace evento