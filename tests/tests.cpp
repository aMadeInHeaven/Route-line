#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#define GET_PID() GetCurrentProcessId()
#else
#include <unistd.h>
#define GET_PID() getpid()
#endif

#include "../lib/UserRequest.h"
#include "../lib/API.h"

using json = nlohmann::json;

std::filesystem::path base_cache_dir = std::filesystem::path(DATAFILES_PATH) / "Cache";

std::filesystem::path GetTestCacheDir() {
    const auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::string test_name = std::string(test_info->test_case_name()) + "_" + 
                           std::string(test_info->name());
    
    for (auto& c : test_name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            c = '_';
        }
    }
    
    int pid = GET_PID();
    std::string unique_name = test_name + "_" + std::to_string(pid);
    auto test_cache_dir = base_cache_dir / unique_name;
    
    std::error_code ec;
    std::filesystem::create_directories(test_cache_dir, ec);
    return test_cache_dir;
}

class GlobalTestEnv : public ::testing::Environment {
public:
    void SetUp() override {
        std::error_code ec;
        std::filesystem::create_directories(base_cache_dir, ec);
        std::cout << "Process " << GET_PID() << ": base cache dir: " << base_cache_dir << std::endl;
    }
};

class MockAPI : public APIwrapper {
public:
    MOCK_METHOD(json, GetData,
        (const std::string&, const std::string&, const std::string&),
        (override));
};

std::string UniqueDate(const std::string& suffix) {
    return "2024-01-01_" + suffix;
}


TEST(UserRequestTest, CallsAPIWhenNoCache) {
    auto test_cache_dir = GetTestCacheDir();
    std::cout << "Test cache dir: " << test_cache_dir << std::endl;
    
    MockAPI api;
    std::string date = UniqueDate("test1");

    json fake_response;
    fake_response["segments"] = json::array();

    EXPECT_CALL(api, GetData("c213", "c2", date))
        .Times(1)
        .WillOnce(testing::Return(fake_response));

    CacheManager cache_manager_ = CacheManager(test_cache_dir.string());
    UserRequest req("Москва", "Санкт-Петербург", date, api, cache_manager_, 1);
    auto res = req.GetPathInfo();
}

TEST(UserRequestTest, UsesCacheInsteadOfAPI) {
    auto test_cache_dir = GetTestCacheDir();
    std::cout << "Test cache dir: " << test_cache_dir << std::endl;
    
    MockAPI api;
    std::string date = UniqueDate("test2");

    EXPECT_CALL(api, GetData(testing::_, testing::_, testing::_))
        .Times(0);

    json cached;
    cached["request_time"] = UserRequest::GetCurTime();
    cached["segments"] = {
        {
            {"has_transfers", false},
            {"departure", "10:00"},
            {"arrival", "12:00"},
            {"start_date", "2024-01-01"},
            {"thread", {
                {"carrier", {{"title", "RZD"}}},
                {"transport_type", "train"},
                {"number", "123"}
            }}
        }
    };

    std::vector<std::string> test_filenames = {
        "simple.json",
        "c213-c2-2024-01-01_test2.json",
        "c213-c2-2024-01-01_test2",
        "c213-c2-test.json",
        "test.json"
    };
    
    for (const auto& filename : test_filenames) {
        std::filesystem::path test_path = test_cache_dir / filename;
        {
            std::ofstream f(test_path);
            if (f.is_open()) {
                f << cached.dump(2);
                f.close();
                std::cout << "Created: " << filename << " - exists: " 
                          << std::filesystem::exists(test_path) << std::endl;
            } else {
                std::cout << "Failed to create: " << filename << std::endl;
            }
        }
    }
    
    std::string cache_filename = "c213-c2-" + date + ".json";
    std::filesystem::path cache_path = test_cache_dir / cache_filename;
    
    std::cout << "\nTrying to create target file: " << cache_filename << std::endl;
    std::cout << "Full path: " << cache_path << std::endl;
    std::cout << "Path string: " << cache_path.string() << std::endl;
    
    std::cout << "Filename characters: ";
    for (char c : cache_filename) {
        std::cout << "'" << c << "' (" << (int)c << ") ";
    }
    std::cout << std::endl;
    
    {
        std::ofstream f(cache_path);
        std::cout << "ofstream is_open: " << f.is_open() << std::endl;
        if (f.is_open()) {
            f << cached.dump(2);
            f.flush();
            f.close();
            std::cout << "File written, checking existence..." << std::endl;
        } else {
            std::cout << "Failed to open file for writing!" << std::endl;
            std::cout << "errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
        }
    }
    
    bool exists = std::filesystem::exists(cache_path);
    std::cout << "File exists after write: " << exists << std::endl;
    
    if (!exists) {
        std::cout << "\nFiles in directory:" << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator(test_cache_dir)) {
            std::cout << "  " << entry.path().filename().string() << std::endl;
        }
    }
    
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "File exists after write: " << std::filesystem::exists(cache_path) << std::endl;
    
    if (std::filesystem::exists(cache_path)) {
        std::cout << "File size: " << std::filesystem::file_size(cache_path) << " bytes" << std::endl;
    }
    
    {
        std::ifstream verify(cache_path);
        ASSERT_TRUE(verify.is_open()) << "File cannot be opened for reading after creation";
        
        std::string content;
        std::getline(verify, content);
        std::cout << "File content first line: " << content << std::endl;
        verify.close();
    }

    ASSERT_TRUE(std::filesystem::exists(cache_path));
    std::cout << "Cache file created: " << cache_path << std::endl;

    CacheManager cache_manager_ = CacheManager(test_cache_dir.string());
    UserRequest req("Москва", "Санкт-Петербург", date, api, cache_manager_, 1);
    auto res = req.GetPathInfo();
}

TEST(UserRequestTest, ParsesSimpleRoute) {
    auto test_cache_dir = GetTestCacheDir();
    
    MockAPI api;
    std::string date = UniqueDate("test3");

    json response;
    response["segments"] = {
        {
            {"has_transfers", false},
            {"departure", "10:00"},
            {"arrival", "12:00"},
            {"start_date", "2024-01-01"},
            {"thread", {
                {"carrier", {{"title", "RZD"}}},
                {"transport_type", "train"},
                {"number", "123"}
            }}
        }
    };

    EXPECT_CALL(api, GetData(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(response));

    CacheManager cache_manager_ = CacheManager(test_cache_dir.string());
    UserRequest req("Москва", "Санкт-Петербург", date, api, cache_manager_, 1);
    auto paths = req.GetPathInfo();

    ASSERT_EQ(paths.size(), 1);
    ASSERT_EQ(paths[0].size(), 1);

    EXPECT_EQ(paths[0][0].company, "RZD");
    EXPECT_EQ(paths[0][0].transport, "train");
}

TEST(UserRequestTest, ParsesRouteWithTransfers) {
    auto test_cache_dir = GetTestCacheDir();
    
    MockAPI api;
    std::string date = UniqueDate("test4");

    json response;
    response["segments"] = {
        {
            {"has_transfers", true},
            {"details", json::array({
                {
                    {"is_transfer", false},
                    {"from", {{"title", "Москва"}}},
                    {"to", {{"title", "Казань"}}},
                    {"departure", "10:00"},
                    {"arrival", "12:00"},
                    {"start_date", "2024-01-01"},
                    {"thread", {
                        {"carrier", {{"title", "RZD"}}},
                        {"transport_type", "train"},
                        {"number", "111"}
                    }}
                },
                {
                    {"is_transfer", false},
                    {"from", {{"title", "Казань"}}},
                    {"to", {{"title", "Уфа"}}},
                    {"departure", "13:00"},
                    {"arrival", "15:00"},
                    {"start_date", "2024-01-01"},
                    {"thread", {
                        {"carrier", {{"title", "RZD"}}},
                        {"transport_type", "train"},
                        {"number", "222"}
                    }}
                }
            })}
        }
    };

    EXPECT_CALL(api, GetData(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(response));

    CacheManager cache_manager_ = CacheManager(test_cache_dir.string());
    UserRequest req("Москва", "Уфа", date, api, cache_manager_, 2);
    auto paths = req.GetPathInfo();

    ASSERT_EQ(paths.size(), 1);
    ASSERT_EQ(paths[0].size(), 2);

    EXPECT_EQ(paths[0][0].from, "Москва");
    EXPECT_EQ(paths[0][1].to, "Уфа");
}

TEST(UserRequestTest, FiltersTooManyTransfers) {
    auto test_cache_dir = GetTestCacheDir();
    
    MockAPI api;
    std::string date = UniqueDate("test5");

    json response;
    response["segments"] = {
        {
            {"has_transfers", true},
            {"transfers", {1,2,3}},
            {"details", json::array()}
        }
    };

    EXPECT_CALL(api, GetData(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(response));

    CacheManager cache_manager_ = CacheManager(test_cache_dir.string());
    UserRequest req("Москва", "Уфа", date, api, cache_manager_, 1);
    auto paths = req.GetPathInfo();

    EXPECT_TRUE(paths.empty());
}


TEST(UserRequestTest, SortPathsByTime) {
    std::vector<std::vector<InfoAboutPath>> paths = {
        { {"A", "B", "X", "2026-03-22", "2026-03-22T10:00:00+0000", "2026-03-22T12:00:00+0000", "Train", "T1"} },

        { 
            {"C", "D", "Y", "2026-03-22", "2026-03-22T08:00:00+0000", "2026-03-22T09:00:00+0000", "Bus", "B1"},
            {"D", "E", "Y", "2026-03-22", "2026-03-22T09:30:00+0000", "2026-03-22T12:30:00+0000", "Bus", "B2"}
        },

        { {"F", "G", "Z", "2026-03-22", "2026-03-22T14:00:00+0000", "2026-03-22T15:30:00+0000", "Plane", "P1"} }
    };

    UtilsUserRequest::SortPathsByTime(paths);

    EXPECT_EQ(paths[0][0].from, "F");
    EXPECT_EQ(paths[1][0].from, "A");
    EXPECT_EQ(paths[2][0].from, "C");

    auto dur0 = UtilsUserRequest::TimeDifference(paths[0][0].departure, paths[0].back().arrival);
    auto dur1 = UtilsUserRequest::TimeDifference(paths[1][0].departure, paths[1].back().arrival);
    auto dur2 = UtilsUserRequest::TimeDifference(paths[2][0].departure, paths[2].back().arrival);

    EXPECT_LT(dur0, dur1);
    EXPECT_LT(dur1, dur2);
}


int main(int argc, char** argv) {
    std::cout << "Process " << GET_PID() << " started" << std::endl;
    
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GlobalTestEnv);
    
    return RUN_ALL_TESTS();
}