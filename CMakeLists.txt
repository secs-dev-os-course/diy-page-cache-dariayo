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