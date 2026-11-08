#pragma once

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace UtilsForCity {
    struct CityNameId {
        std::string n;
        std::string id;
    };

    int Levenshtein(const std::string& str1, const std::string& str2);
    std::u16string StrToLowerCase(std::u16string str);
}; // UtilsForCity;

class City {
public:
    using json = nlohmann::json;

    City() = default;
    explicit City(const std::string& name);

    City(const City& other) = default;
    City& operator=(const City& other) = default;
    ~City() = default;

    static UtilsForCity::CityNameId FindCityByName(const std::string& city_name);
    static std::string GetCityId(const std::string& city_name);

    const std::string GetName() const { return name_; }
    const std::string GetId() const { return city_id_; }
private:
    std::string name_;
    std::string city_id_;

    static json station_codes_;
    static bool station_codes_init_;
    
    static void LoadStationCodes(const std::string& path);
};
