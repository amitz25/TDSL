cmake_minimum_required(VERSION 2.8)
project(TDSL)
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread -O3")
find_package (Threads)
find_package(Boost 1.50 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES tskiplist/Index.cpp tskiplist/WriteSet.cpp tskiplist/TSkipList.cpp tskiplist/skiplist/skiplist.cc)

add_library(tdsl ${SOURCE_FILES})

add_executable(tdsl-test experiments.cpp ${SOURCE_FILES})
target_link_libraries (tdsl-test ${CMAKE_THREAD_LIBS_INIT})

add_executable(benchmark bench/main.cc bench/timehelper.cc bench/allocator.cc bench/assert.cc ${SOURCE_FILES})
