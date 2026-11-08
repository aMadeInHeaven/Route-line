add_executable(MyTests tests.cpp TestsCity.cpp)

target_compile_definitions(MyTests
    PUBLIC DATAFILES_PATH="${CMAKE_BINARY_DIR}/DataFiles"
)

target_link_libraries(MyTests
    PRIVATE
    CoolLibFindingPath
    gtest
    gmock
    #gtest_main
)

add_custom_target(clean_cache
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/DataFiles/Cache
    COMMENT "Cleaning cache directory before tests"
)

add_dependencies(MyTests clean_cache)

include(GoogleTest)
gtest_discover_tests(MyTests)