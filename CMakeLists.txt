cmake_minimum_required(VERSION 3.6)
project(TDSL)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES tskiplist/Index.cpp tskiplist/WriteSet.cpp tskiplist/TSkipList.cpp tskiplist/skiplist/skiplist.cc)
add_library(tdsl ${SOURCE_FILES})
add_executable(tdsl-test experiments.cpp ${SOURCE_FILES})