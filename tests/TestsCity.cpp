#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <utility>

#include "../lib/City.h"
#include "../lib/CustomExeptions.h"

using json = nlohmann::json;

class CityTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::path station_codes_path = std::filesystem::path(DATAFILES_PATH) / "station_codes.json";
        if (!std::filesystem::exists(station_codes_path)) {
            std::cout << "Warning: station_codes.json not found at " << station_codes_path << std::endl;
        }
    }
};

TEST_F(CityTest, CreateCityWithExactName) {
    EXPECT_NO_THROW({
        City moscow("Москва");
        EXPECT_EQ(moscow.GetName(), "Москва");
        EXPECT_EQ(moscow.GetId(), "c213");
    });
    
    EXPECT_NO_THROW({
        City spb("Санкт-Петербург");
        EXPECT_EQ(spb.GetName(), "Санкт-Петербург");
        EXPECT_EQ(spb.GetId(), "c2");
    });
    
    EXPECT_NO_THROW({
        City kazan("Казань");
        EXPECT_EQ(kazan.GetName(), "Казань");
        EXPECT_EQ(kazan.GetId(), "c43");
    });
}

TEST_F(CityTest, CreateCityWithDifferentCase) {
    EXPECT_NO_THROW({
        City moscow_lower("москва");
        EXPECT_EQ(moscow_lower.GetName(), "Москва");
        EXPECT_EQ(moscow_lower.GetId(), "c213");
    });
    
    EXPECT_NO_THROW({
        City spb("Санкт-Петербург");
        EXPECT_EQ(spb.GetName(), "Санкт-Петербург");
        EXPECT_EQ(spb.GetId(), "c2");
    });
}

TEST_F(CityTest, AutocorrectSimilarNames) {
    EXPECT_NO_THROW({
        City moscow("Москвa");
        EXPECT_EQ(moscow.GetName(), "Москва");
        EXPECT_EQ(moscow.GetId(), "c213");
    });
    
    EXPECT_NO_THROW({
        City spb("Санкт-Питербург");
        EXPECT_EQ(spb.GetName(), "Санкт-Петербург");
        EXPECT_EQ(spb.GetId(), "c2");
    });
    
    EXPECT_NO_THROW({
        City kazan("Казаньь");
        EXPECT_EQ(kazan.GetName(), "Казань");
        EXPECT_EQ(kazan.GetId(), "c43");
    });
}

TEST_F(CityTest, InvalidCityName) {
    EXPECT_THROW({
        City invalid("Абобаградфффффф");
    }, CustomExceptions::CityException);
    
    EXPECT_THROW({
        City invalid("SomeRandomCity123");
    }, CustomExceptions::CityException);
    
    EXPECT_THROW({
        City invalid("Москвааааааа");
    }, CustomExceptions::CityException);
}

TEST_F(CityTest, FindCityByNameStaticMethod) {
    auto result = City::FindCityByName("Москва");
    EXPECT_EQ(result.n, "Москва");
    EXPECT_EQ(result.id, "c213");
    
    result = City::FindCityByName("санкт-петербург");
    EXPECT_EQ(result.n, "Санкт-Петербург");
    EXPECT_EQ(result.id, "c2");
    
    result = City::FindCityByName("Казань");
    EXPECT_EQ(result.n, "Казань");
    EXPECT_EQ(result.id, "c43");
    
    result = City::FindCityByName("Казаньа");
    EXPECT_EQ(result.n, "Казань");
    EXPECT_EQ(result.id, "c43");
}

TEST_F(CityTest, GetCityIdStaticMethod) {
    std::string id = City::GetCityId("Москва");
    EXPECT_EQ(id, "c213");
    
    id = City::GetCityId("Санкт-Петербург");
    EXPECT_EQ(id, "c2");
    
    id = City::GetCityId("Казань");
    EXPECT_EQ(id, "c43");
    
    EXPECT_THROW({
        City::GetCityId("НесуществующийГород");
    }, CustomExceptions::CityException);
}

TEST_F(CityTest, CopyOperations) {
    City moscow("Москва");
    City copy_constructor(moscow);
    
    EXPECT_EQ(copy_constructor.GetName(), moscow.GetName());
    EXPECT_EQ(copy_constructor.GetId(), moscow.GetId());
    
    City copy_assignment;
    copy_assignment = moscow;
    
    EXPECT_EQ(copy_assignment.GetName(), moscow.GetName());
    EXPECT_EQ(copy_assignment.GetId(), moscow.GetId());
}

TEST(UtilsForCityTest, LevenshteinDistance) {
    EXPECT_EQ(UtilsForCity::Levenshtein("Москва", "Москва"), 0);
    EXPECT_EQ(UtilsForCity::Levenshtein("Москва", "МоскваВ"), 1);
    EXPECT_EQ(UtilsForCity::Levenshtein("Москва", "Москваа"), 1);
    EXPECT_EQ(UtilsForCity::Levenshtein("Москва", "Моск"), 2);
    EXPECT_EQ(UtilsForCity::Levenshtein("Санкт-Петербург", "Санкт-Питербург"), 1);
    EXPECT_EQ(UtilsForCity::Levenshtein("", "Москва"), 6);
    EXPECT_EQ(UtilsForCity::Levenshtein("Москва", ""), 6);
    EXPECT_EQ(UtilsForCity::Levenshtein("abc", "abc"), 0);
    EXPECT_EQ(UtilsForCity::Levenshtein("abc", "abd"), 1);
}

TEST_F(CityTest, CityWithHyphen) {
    EXPECT_NO_THROW({
        City rostov("Ростов-на-Дону");
        EXPECT_EQ(rostov.GetName(), "Ростов-на-Дону");
        EXPECT_EQ(rostov.GetId(), "c39");
    });
}

TEST_F(CityTest, SpecialCharactersInName) {
    EXPECT_NO_THROW({
        City city("Москва!!!");
        EXPECT_EQ(city.GetId(), "c213");
    });
}

TEST_F(CityTest, AutocorrectAtMaxDistance) {
    EXPECT_THROW({
        City city("Москвааааа");
    }, CustomExceptions::CityException);
    
    EXPECT_NO_THROW({
        City city("Масква");
        EXPECT_EQ(city.GetId(), "c213");
    });
}

TEST_F(CityTest, StaticMethodsWithoutInstance) {
    auto name_id = City::FindCityByName("Москва");
    EXPECT_EQ(name_id.id, "c213");
    
    std::string id = City::GetCityId("Санкт-Петербург");
    EXPECT_EQ(id, "c2");
}

TEST_F(CityTest, RepeatedFindCityByName) {
    auto result1 = City::FindCityByName("Москва");
    auto result2 = City::FindCityByName("Москва");
    
    EXPECT_EQ(result1.n, result2.n);
    EXPECT_EQ(result1.id, result2.id);
}

TEST_F(CityTest, CreateCityFromFindResult) {
    auto result = City::FindCityByName("Казань");
    City city(result.n);
    
    EXPECT_EQ(city.GetName(), "Казань");
    EXPECT_EQ(city.GetId(), "c43");
}

TEST_F(CityTest, AllKnownCities) {
    std::vector<std::pair<std::string, std::string>> cities = {
        {"Москва", "c213"},
        {"Санкт-Петербург", "c2"},
        {"Казань", "c43"},
        {"Нижний Новгород", "c23243"},
        {"Екатеринбург", "c54"},
        {"Новосибирск", "c65"},
        {"Красноярск", "c62"},
        {"Владивосток", "c75"},
        {"Сочи", "c239"},
        {"Ростов-на-Дону", "c39"}
    };
    
    for (const auto& [name, expected_id] : cities) {
        EXPECT_NO_THROW({
            City city(name);
            EXPECT_EQ(city.GetId(), expected_id) << "Failed for city: " << name;
            EXPECT_EQ(city.GetName(), name) << "Failed for city: " << name;
        });
    }
}

TEST_F(CityTest, DifferentSpellings) {
    std::vector<std::string> spellings = {
        "Санкт-Петербург",
        "Санкт-Питербург",
        "Санкт Петербург",
        "Санкт-Петербург"
    };
    
    for (const auto& spelling : spellings) {
        testing::internal::CaptureStderr();
        EXPECT_NO_THROW({
            City city(spelling);
            EXPECT_EQ(city.GetId(), "c2");
        });
        testing::internal::GetCapturedStderr();
    }
}

TEST_F(CityTest, MultipleSpacesInName) {
    EXPECT_NO_THROW({
        City city("Санкт-Петербург");
        EXPECT_EQ(city.GetId(), "c2");
    });
}
