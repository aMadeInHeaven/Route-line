#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <exception>
#include <vector>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>

#include "City.h"
#include "UserRequest.h"


void UtilsUserRequest::SortPathsByTime(std::vector<std::vector<InfoAboutPath>>& paths) {
    std::sort(paths.begin(), paths.end(), 
        [](const std::vector<InfoAboutPath>& a, const std::vector<InfoAboutPath>& b) -> bool { 
            long long a_dur = 0;
            long long b_dur = 0;

            if (a.size() == 1) {
                a_dur = TimeDifference(a[0].departure, a[0].arrival);
            } else if (a.size() >= 2) {
                a_dur = TimeDifference(a[0].departure, a.back().arrival);
            }

            if (b.size() == 1) {
                b_dur = TimeDifference(b[0].departure, b[0].arrival);
            } else if (b.size() >= 2) {
                b_dur = TimeDifference(b[0].departure, b.back().arrival);
            }

            return a_dur < b_dur;
        });
}

long long UtilsUserRequest::TimeDifference(const std::string& departure, const std::string& arrival) {
    std::chrono::sys_seconds departure_seconds;
    std::chrono::sys_seconds arrival_seconds;

    std::istringstream depart_stream(departure);
    std::istringstream arrival_stream(arrival);

    depart_stream >> std::chrono::parse("%Y-%m-%dT%H:%M:%S%z", departure_seconds);
    arrival_stream >> std::chrono::parse("%Y-%m-%dT%H:%M:%S%z", arrival_seconds);

    return (arrival_seconds - departure_seconds).count();
}

long long UserRequest::GetCurTime() {
    auto cur_time = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::seconds>(cur_time).time_since_epoch().count();
}

nlohmann::json UserRequest::Request() {
    std::optional<json> paths = cache_manager_.LoadCacheFile();
    
    if (!paths) {
        paths = api_.GetData(
            from_.GetId(),
            to_.GetId(),
            date_
        );

        (*paths)["request_time"] = GetCurTime();
        cache_manager_.WriteCacheFile(*paths);
    }

    return *paths;
}

std::vector<std::vector<InfoAboutPath>> UserRequest::GetPathInfo() {
    std::vector<std::vector<InfoAboutPath>> paths;
    json paths_json = Request();

    if (!paths_json.contains("segments")) {
        return paths;
    }

    std::vector<InfoAboutPath> path;
    for (auto& segment : paths_json["segments"]) {
        try {
            if (segment.contains("transport_types") && 
                segment["transport_types"].size() > max_transfer_ + 1) 
            {
                continue;
            }

            if (segment.contains("transfers") && 
                segment["transfers"].size() > max_transfer_) 
            {
                continue;
            }

            path.clear();
            if (!segment.value("has_transfers", false)) {
                InfoAboutPath info;

                info.from = from_.GetName();
                info.to = to_.GetName();

                info.company = segment["thread"]["carrier"].value("title", "unknown");

                info.departure = segment["departure"];
                info.arrival = segment["arrival"];
                info.date = segment["start_date"];

                info.transport = segment["thread"].value("transport_type", "unknown");
                info.transport_numbers = segment["thread"].value("number", "unknown");

                path.push_back(info);
            } else if (segment.contains("details")) {
                for (auto& d : segment["details"]) {

                    if (d.value("is_transfer", false)) {
                        continue;
                    }

                    InfoAboutPath info;

                    info.from = d["from"].value("title", "unknown");
                    info.to = d["to"].value("title", "unknown");
                    info.company = d["thread"]["carrier"].value("title", "unknown");

                    info.departure = d["departure"];
                    info.arrival = d["arrival"];
                    info.date = d["start_date"];

                    info.transport = d["thread"].value("transport_type", "unknown");
                    info.transport_numbers = d["thread"].value("number", "unknown");

                    path.push_back(info);
                }
            }

            if (!path.empty()) {
                paths.push_back(path);
            }
        } catch (const std::exception& e) {
            std::cerr << "Parsing segment error: " << e.what() << '\n';
            continue;
        }
    }

    UtilsUserRequest::SortPathsByTime(paths);
    return paths;
}
