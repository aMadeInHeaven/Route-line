#pragma once

#include <exception>
#include <string>

namespace CustomExceptions {

    class ExceptionBase : public std::exception {
        public:
            explicit ExceptionBase(const std::string& error_msg) : error_message_(error_msg) {}

            const char* what() const noexcept override {
                return error_message_.c_str();
            }
        protected:
            std::string error_message_;
    };

    class ParseJSONexception : public ExceptionBase {
        public:
            explicit ParseJSONexception(const std::string& error_msg) : ExceptionBase("JSON parse error: " + error_msg) {}
    };

    class CityException : public ExceptionBase {
        public:
            explicit CityException(const std::string& city_name) : ExceptionBase("Wrong city name: " + city_name) {}
    };

    class CacheException : public ExceptionBase {
        public:
            explicit CacheException(const std::string& error_msg) : ExceptionBase("Cache error: " + error_msg) {}
    };

    class NetworkException : public ExceptionBase {
        public:
            explicit NetworkException(const std::string& msg) : ExceptionBase("Network error: " + msg) {}
    };

    class HttpException : public ExceptionBase {
        public:
            explicit HttpException(int code) : ExceptionBase("HTTP error: status code: " + std::to_string(code)) {}
    };

    class ConfigException : public ExceptionBase {
        public:
            explicit ConfigException(const std::string& error_msg) : ExceptionBase(error_msg) {}
    };

} // CustomExceptions
