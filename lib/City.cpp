#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <climits>
#include <algorithm>
#include <codecvt>

#include "City.h"
#include "CustomExeptions.h"

std::u16string UtilsForCity::StrToLowerCase(std::u16string str) {
    for (char16_t& c : str) {
        if (c >= u'A' && c <= u'Z') {
            c = c + (u'a' - u'A');
        } else if (c >= u'А' && c <= u'Я') {
            c = c + (u'а' - u'А');
        } else if (c == u'Ё') {
            c = u'ё';
        }
    }
    return str;
}

int UtilsForCity::Levenshtein(const std::string& str1, const std::string& str2) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    
    std::u16string str1_u16 = converter.from_bytes(str1);
    std::u16string str2_u16 = converter.from_bytes(str2);

    str1_u16 = StrToLowerCase(str1_u16);
    str2_u16 = StrToLowerCase(str2_u16);

    size_t n = str1_u16.size();
    size_t m = str2_u16.size();

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1));

    for (size_t i = 0; i <= n; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= m; ++j) dp[0][j] = j;

    int cost = 0;
    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            cost = (str1_u16[i - 1] == str2_u16[j - 1]) ? 0 : 1;

            dp[i][j] = std::min(dp[i - 1][j] + 1, std::min(dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost));
        }
    }

    return dp[n][m];
}

nlohmann::json City::station_codes_;
bool City::station_codes_init_ = false;

void City::LoadStationCodes(const std::string& path) {
    if (!station_codes_init_) {
        std::ifstream station_codes(path);

        if (!station_codes.is_open()) {
            throw std::runtime_error("Can't open file with station code, sorry not today\n");
        }

        station_codes >> station_codes_;
        station_codes.close();

        station_codes_init_ = true;
    }
}

UtilsForCity::CityNameId City::FindCityByName(const std::string& city_name) {
    auto path = std::filesystem::path(DATAFILES_PATH) / "station_codes.json";
    LoadStationCodes(path.string());
    if (station_codes_.contains(city_name)) {
        return {city_name, station_codes_[city_name]};
    }

    std::string goal_city = "";
    int best_dist = INT_MAX;

    int dist = 0;
    for (auto it = station_codes_.begin(); it != station_codes_.end(); ++it) {
        dist = UtilsForCity::Levenshtein(it.key(), city_name);

        if (dist < best_dist) {
            best_dist = dist;
            goal_city = it.key();
        }
    }

    if (best_dist <= 3) {
        return {goal_city, station_codes_[goal_city]};
    }

    throw CustomExceptions::CityException(city_name);
}

std::string City::GetCityId(const std::string& city_name) {
    return FindCityByName(city_name).id;
}

City::City(const std::string& name) {
    UtilsForCity::CityNameId name_id = FindCityByName(name);
    name_ = name_id.n;
    city_id_= name_id.id;
}