include(FetchContent)

FetchContent_Declare(
    googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
)

FetchContent_MakeAvailable(googletest)

add_executable(key_value_store_test)
target_sources(key_value_store_test
    PRIVATE
        KeyValueStoreTest.cpp
        KeyValueStorePerformanceTest.cpp)

target_link_libraries(key_value_store_test
    PRIVATE
        key_value_store
        gmock_main
)
