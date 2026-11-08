#pragma once

#include "UserRequest.h"

#include <filesystem>
#include <vector>
#include <string>

class TUI {
public:
    TUI() : cache_manager_((std::filesystem::path(DATAFILES_PATH) / "Cache").string()) {}
    TUI(const TUI& other) = delete;
    TUI& operator=(const TUI& other) = delete;

    ~TUI() = default;

    void Init();
private:
    CacheManager cache_manager_;
    std::vector<std::vector<InfoAboutPath>> paths_;
    int max_paths_ammount_ = 20;

    std::string error_message_;

    void GetPathsInfo(const std::string& from, const std::string& to, 
                      const std::string& date, int transfers);

    std::vector<std::string> FormatPaths();
    std::vector<std::string> FormatError();
    std::vector<std::string> FormatDetails(int index);
};