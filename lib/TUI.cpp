#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/util/ref.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "UserRequest.h"
#include "API.h"
#include "TUI.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

std::unordered_map<std::string, std::string> vehicals = {
    {"plane", "самолет"},
    {"train", "поезд"},
    {"suburban", "электричка"},
    {"bus", "автобус"},
    {"water" , "водный транспорт"},
    {"helicopter", "вертолет"}
};

std::vector<std::string> TUI::FormatPaths() {
    std::vector<std::string> result;

    for (int i = 0; i < paths_.size(); ++i) {
        const std::vector<InfoAboutPath>& path = paths_[i];

        if (path.empty())
            continue;

        std::string line = std::to_string(i + 1) + ". ";

        line += path.front().from;

        for (const InfoAboutPath& segment : path) {
            line += " → " + segment.to;
        }

        line += " - " + std::to_string(path.size() - 1) + " transfers";

        long long duration =
            UtilsUserRequest::TimeDifference(
                path.front().departure,
                path.back().arrival
            );

        long long hours = duration / 3600;
        long long minutes = (duration % 3600) / 60;

        line += " - " + std::to_string(hours) + "h " + std::to_string(minutes) + "m";

        result.push_back(line);
    }

    return result;
}

std::vector<std::string> TUI::FormatError() {
    std::vector<std::string> result;
    
    if (error_message_.empty()) {
        return result;
    }
    
    result.push_back("========== ERROR ==========");
    result.push_back("Failed to get routes:");
    result.push_back("");
    result.push_back(error_message_);
    result.push_back("");
    result.push_back("Please check:");
    result.push_back("- Connection to server");
    result.push_back("- Input data format (especially city names)");
    
    return result;
}

std::vector<std::string> TUI::FormatDetails(int index) {
    std::vector<std::string> result;

    if (index >= paths_.size())
        return result;

    const std::vector<InfoAboutPath>& path = paths_[index];

    if (path.empty())
        return result;

    result.push_back("========== Route " + std::to_string(index + 1) + " ==========");

    std::string route_line = path.front().from;
    for (const InfoAboutPath& segment : path) {
        route_line += " → " + segment.to;
    }
    result.push_back(route_line);

    result.push_back("Transfers: " + std::to_string(path.size() - 1));
    result.push_back("");

    int segment_index = 1;
    for (const InfoAboutPath& segment : path) {
        result.push_back("[" + std::to_string(segment_index++) + "] " +
                         segment.from + " → " + segment.to);

        std::string transport = "🚌 Transport: " + vehicals[segment.transport];
        if (!segment.transport_numbers.empty()) {
            transport += " (" + segment.transport_numbers + ")";
        }
        result.push_back("    " + transport);

        std::string seg_departure = segment.departure;
        std::replace(seg_departure.begin(), seg_departure.end(), 'T', ' ');

        std::string seg_arrival = segment.arrival;
        std::replace(seg_arrival.begin(), seg_arrival.end(), 'T', ' ');

        result.push_back("    🏢 Company: " + segment.company);
        result.push_back("    🕒 Time: " + seg_departure + " → " + seg_arrival);
        result.push_back("    📅 Date: " + segment.date);
        result.push_back("----------------------------------------");
    }

    long long duration =
        UtilsUserRequest::TimeDifference(
            path.front().departure,
            path.back().arrival
        );

    long long hours = duration / 3600;
    long long minutes = (duration % 3600) / 60;
    long long seconds = duration % 60;

    result.push_back("Total time: " +
        std::to_string(hours) + "h " +
        std::to_string(minutes) + "m " +
        std::to_string(seconds) + "s");

    return result;
}

void TUI::Init() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    std::string from;
    std::string to;
    std::string date;
    std::string transfers;

    int selected_route = 0;
    bool show_details = false;

    auto input_from = Input(&from, "Departure city");
    auto input_to = Input(&to, "Arrival city");
    auto input_date = Input(&date, "Date: YYYY-MM-DD");
    auto input_transfers = Input(&transfers, "Transfers limit");

    auto exit_button = Button("Exit", screen.ExitLoopClosure());

    auto search_button = Button("Find paths", [&] {
        if (transfers.empty())
            transfers = "0";

        error_message_.clear();

        GetPathsInfo(from, to, date, std::stoi(transfers));

        selected_route = 0;
        show_details = false;
    });

    auto left_container = Container::Vertical({
        input_from,
        input_to,
        input_date,
        input_transfers,
        search_button,
        exit_button,
    });

    auto inputs_panel = Renderer(left_container, [&] {
        return vbox({
            hbox({text("From: ") | size(WIDTH, EQUAL, 12), input_from->Render() | flex}),
            hbox({text("To:   ") | size(WIDTH, EQUAL, 12), input_to->Render() | flex}),
            hbox({text("Date: ") | size(WIDTH, EQUAL, 12), input_date->Render() | flex}),
            hbox({text("Transfers: ") | size(WIDTH, EQUAL, 12), input_transfers->Render() | flex}),
            filler(),
            search_button->Render() | border | center,
            filler(),
            exit_button->Render() | center
        }) | border | size(WIDTH, EQUAL, 40);
    });

    auto output_panel = Renderer([&] {
        Elements content;

        if (!error_message_.empty()) {
            std::vector<std::string> error_lines = FormatError();
            for (std::string& line : error_lines) {
                auto text_element = text(line);
                content.push_back(text_element);
            } 
        } else if (paths_.empty()) {
            content.push_back(text("No data. Press 'Find paths'"));
        } else if (!show_details) {
            std::vector<std::string> list = FormatPaths();

            for (int i = 0; i < list.size(); ++i) {
                auto line = text(list[i]);

                if (i == selected_route)
                    line = line | inverted;

                content.push_back(line);
            }
        } else {
            std::vector<std::string> details = FormatDetails(selected_route);

            for (std::string& line : details) {
                content.push_back(text(line));
            }
        }

        return vbox(content) | border | flex;
    });

    auto container = Container::Horizontal({
        left_container,
        output_panel
    });

    auto renderer = Renderer(container, [&] {
        return hbox({
            inputs_panel->Render(),
            output_panel->Render() | flex
        });
    })
    | CatchEvent([&](Event event) {

        auto list = FormatPaths();

        if (!show_details && !list.empty()) {
            if (event == Event::ArrowDown) {
                selected_route = std::min(selected_route + 1, (int)list.size() - 1);
                return true;
            }
            if (event == Event::ArrowUp) {
                selected_route = std::max(selected_route - 1, 0);
                return true;
            }
        }

        if (event == Event::Return && !list.empty()) {
            show_details = !show_details;
            return true;
        }

        return false;
    });

    screen.Loop(renderer);
}

void TUI::GetPathsInfo(const std::string& from, const std::string& to, 
                       const std::string& date, int transfers) {
    try {
        RealAPIwrapper api;
        UserRequest a(from, to, date, api, cache_manager_, transfers);
        paths_ = a.GetPathInfo();

        paths_.erase(
            std::remove_if(paths_.begin(), paths_.end(),
                [](const auto& p) { return p.empty(); }),
            paths_.end()
        );

        if (paths_.size() > max_paths_ammount_) {
            paths_.resize(max_paths_ammount_);
        }
    }  catch (const CustomExceptions::ConfigException& e) {
        paths_.clear(); 
        error_message_ = "[Config error] " + std::string(e.what());
    } catch (const CustomExceptions::CityException& e) {
        paths_.clear(); 
        error_message_ = "[City error] " + std::string(e.what());
    } catch (const CustomExceptions::NetworkException& e) {
        paths_.clear(); 
        error_message_ = "[Network error] " + std::string(e.what());
    } catch (const CustomExceptions::HttpException& e) {
        paths_.clear(); 
        error_message_ = "[HTTP error] " + std::string(e.what());
    } catch (const CustomExceptions::ParseJSONexception& e) {
        paths_.clear(); 
        error_message_ = "[JSON error] " + std::string(e.what());
    } catch (const CustomExceptions::CacheException& e) {
        paths_.clear(); 
        error_message_ = "[Cache error] " + std::string(e.what());
    } catch (const std::exception& e) {
        paths_.clear(); 
        error_message_ = "[Unknown error] " + std::string(e.what());
    } catch (...) {
        paths_.clear(); 
        error_message_ = "[Unknown error]";
    }
}
