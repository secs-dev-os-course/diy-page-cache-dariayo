cmake_minimum_required(VERSION 3.27)
project(BlockCache)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    app/block_cache.cpp
    app/bench1.cpp
    app/utils.cpp
)

include_directories(include)

add_executable(bench1 ${SOURCES})

target_link_libraries(bench1)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    test/block_cache_test.cpp
)

add_executable(block_cache_test ${TEST_SOURCES} app/block_cache.cpp)
target_link_libraries(block_cache_test GTest::GTest GTest::Main pthread)
add_test(NAME BlockCacheTests COMMAND block_cache_test)
