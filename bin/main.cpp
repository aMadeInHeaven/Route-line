#include "lib/TUI.h"

int main() {
    TUI t;
    
    try {
        t.Init();
        return 0;
    } catch (const CustomExceptions::ConfigException& e) {
        std::cerr << "[Config error] " << e.what() << '\n';
    } catch (const CustomExceptions::CityException& e) {
        std::cerr << "[City error] " << e.what() << '\n';
    } catch (const CustomExceptions::NetworkException& e) {
        std::cerr << "[Network error] " << e.what() << '\n';
    } catch (const CustomExceptions::HttpException& e) {
        std::cerr << "[HTTP error] " << e.what() << '\n';
    } catch (const CustomExceptions::ParseJSONexception& e) {
        std::cerr << "[JSON error] " << e.what() << '\n';
    } catch (const CustomExceptions::CacheException& e) {
        std::cerr << "[Cache error] " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "[Unknown error] " << e.what() << '\n';
    }

    return 1;
}
