cmake_minimum_required(VERSION 2.8)
project(TDSL)
set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11) 

set(SOURCE_FILES tskiplist/Index.cpp tskiplist/WriteSet.cpp tskiplist/TSkipList.cpp tskiplist/skiplist/skiplist.cc)

find_package(Boost 1.50 COMPONENTS system filesystem REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_library(tdsl ${SOURCE_FILES})
add_executable(tdsl-test sanityCheck.cpp ${SOURCE_FILES})
add_executable(benchmark bench/main.cc bench/timehelper.cc bench/allocator.cc bench/assert.cc ${SOURCE_FILES})
