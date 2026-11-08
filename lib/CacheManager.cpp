#include <iostream>
#include <ios>
#include <exception>
#include <optional>
#include <fstream>
#include <deque>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "CacheManager.h"
#include "UserRequest.h"

namespace fs = std::filesystem;

CacheManager::CacheManager(const std::string& cache_dir) : cache_directory_(cache_dir) {
    if (!fs::exists(cache_directory_)) {
        return;
    }

    for (const auto& file : fs::directory_iterator(cache_directory_)) {
        std::ifstream f(file.path());
        if (f.is_open()) {
            try {
                json file_json;
                f >> file_json;
                cache_files_.push_back({file.path().string(), file_json});
            } catch (...) {
                continue;
            }
        }

        if (cache_files_.size() >= max_cache_files_size_) {
            break;
        }
    }
}

CacheManager::~CacheManager() {
    if (fs::exists(cache_directory_)) {
        for (const auto& file : fs::directory_iterator(cache_directory_)) {
            fs::remove_all(file.path());
        }
    } else {
        fs::create_directories(cache_directory_);
    }

    for (const auto& element : cache_files_) {
        std::ofstream f(element.cache_file_name);
        if (f.is_open()) {
            f << element.data;
        }
    }
}

void CacheManager::WriteCacheFile(const nlohmann::json& file) {
    auto iter = cache_files_.begin();

    while (iter != cache_files_.end() && iter->cache_file_name != cache_file_name_) {
        ++iter;
    }

    if (iter != cache_files_.end()) {
        iter->data = file;
    } else {
        if (cache_files_.size() >= max_cache_files_size_) {
            cache_files_.pop_front();
        }
        cache_files_.push_back({cache_file_name_, file});
    }
}

std::optional<nlohmann::json> CacheManager::LoadCacheFile() {
    auto iter = cache_files_.begin();

    while (iter != cache_files_.end() && iter->cache_file_name != cache_file_name_) {
        ++iter;
    }

    if (iter == cache_files_.end()) {
        return {};
    }

    long long cache_file_time = iter->data.value("request_time", 0);
    long long cur_time = UserRequest::GetCurTime();

    if (cur_time < cache_file_time) {
        return {};
    }

    if (cur_time - cache_file_time <= max_time_dif_) {
        return iter->data;
    }

    cache_files_.erase(iter);
    return {};
}
