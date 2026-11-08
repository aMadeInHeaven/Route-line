#pragma once

#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "CustomExeptions.h"


class APIwrapper {
public:
    virtual ~APIwrapper() = default;

    virtual nlohmann::json GetData(const std::string& from,
                                   const std::string& to,
                                   const std::string& date) = 0;
};

class RealAPIwrapper : public APIwrapper {
public:
    nlohmann::json GetData(const std::string& from,
                           const std::string& to,
                           const std::string& date) override {
        std::string api_key = GetApiKey();

        cpr::Response r = cpr::Get(cpr::Url{"https://api.rasp.yandex-net.ru/v3.0/search"},
                      cpr::Parameters{{"apikey", api_key}, 
                                      {"lang", "ru_RU"}, 
                                      {"from", from},
                                      {"to", to},
                                      {"format", "json"},
                                      {"date", date},
                                      {"transfers", "true"}});

        if (r.error.code != cpr::ErrorCode::OK) {
            throw CustomExceptions::NetworkException(r.error.message);
        }

        if (r.status_code != 200) {
            throw CustomExceptions::HttpException(r.status_code);
        }

        nlohmann::json paths;

        try {
            paths = nlohmann::json::parse(r.text);
        } catch (const nlohmann::json::parse_error& e) {
            throw CustomExceptions::ParseJSONexception(e.what());
        }

        return paths;
    }
private:
    std::string GetApiKey() {
        const char* api_key = std::getenv("API_KEY");
        if (api_key) {
            return std::string(api_key);
        }

        throw CustomExceptions::ConfigException("No API key");
    }
};
