#pragma once

#include <string>
#include <optional>
#include <deque>
#include <nlohmann/json.hpp>

class CacheManager {
public:
    using json = nlohmann::json;

    struct Element {
        std::string cache_file_name;
        json data;  
    };

    CacheManager() = default;
    explicit CacheManager(const std::string& cache_dir);

    CacheManager(const CacheManager& other) = default;
    CacheManager& operator=(const CacheManager& other) = default;
    ~CacheManager();

    void WriteCacheFile(const json& file);
    std::optional<json> LoadCacheFile();

    std::string GetCacheDirectory() const { return cache_directory_; }
    void SetCacheFile(const std::string& cache_file) { cache_file_name_ = cache_file; }
private:
    std::string cache_directory_;
    std::deque<Element> cache_files_;
    const size_t max_cache_files_size_ = 10;
    static const long long max_time_dif_ = 600;

    std::string cache_file_name_;
};