add_subdirectory(tests)

add_library(key_value_store)

target_sources(key_value_store
    PRIVATE
        KeyValueStore.cpp
)

target_compile_features(key_value_store PUBLIC cxx_std_17)

target_include_directories(key_value_store PUBLIC include)
