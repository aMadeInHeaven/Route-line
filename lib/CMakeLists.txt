add_library(CoolLibFindingPath STATIC UserRequest.cpp City.cpp CacheManager.cpp TUI.cpp)

target_compile_definitions(CoolLibFindingPath
    PUBLIC DATAFILES_PATH="${CMAKE_BINARY_DIR}/DataFiles"
)

target_link_libraries(CoolLibFindingPath 
    PUBLIC ftxui::component
    PUBLIC ftxui::dom
    PUBLIC ftxui::screen
)

target_link_libraries(CoolLibFindingPath PUBLIC cpr::cpr)
target_link_libraries(CoolLibFindingPath PUBLIC nlohmann_json::nlohmann_json)

