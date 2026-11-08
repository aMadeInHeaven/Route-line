#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "CacheManager.h"
#include "City.h"
#include "API.h"

struct InfoAboutPath {
    std::string from;

    std::string to;

    std::string company;

    std::string date;
    std::string departure;
    std::string arrival;

    std::string transport;
    std::string transport_numbers;
};

namespace UtilsUserRequest {
    void SortPathsByTime(std::vector<std::vector<InfoAboutPath>>& paths);
    long long TimeDifference(const std::string& departure, const std::string& arrival);
}

class UserRequest {
public:
    using json = nlohmann::json;
    
    explicit UserRequest(const std::string& from, const std::string& to, 
                         const std::string& date, APIwrapper& api, CacheManager& manager,
                                                        int max_transfer = 1) : 
                                                        cache_manager_(manager), api_(api) {
        from_ = City(from);
        to_ = City(to);

        date_ = date;
        max_transfer_ = max_transfer;

        std::filesystem::path cache_file = std::filesystem::path(cache_manager_.GetCacheDirectory()) / (from_.GetId() + "-" + to_.GetId() + "-" + date_ + ".json");

        cache_manager_.SetCacheFile(cache_file.string());
    }

    std::vector<std::vector<InfoAboutPath>> GetPathInfo();
    
    UserRequest(const UserRequest& other) = delete;
    UserRequest& operator=(const UserRequest& other) = delete;
    ~UserRequest() = default;

    static long long GetCurTime();
private:
    City from_;
    City to_;
    std::string date_;
    int max_transfer_;
    CacheManager& cache_manager_;
    APIwrapper& api_;

    json Request();
};
