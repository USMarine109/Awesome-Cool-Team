cmake_minimum_required(VERSION 4.3)
project(Awesome_Cool_Team_momentum)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(include src)

add_executable(Awesome_Cool_Team_momentum src/main.cpp
        src/Load_Dataset.cpp
        src/Load_Dataset.h
        src/Segment_Tree.cpp
        src/Segment_Tree.h
        src/Fenwick_Tree.cpp
        src/Fenwick_Tree.h)
add_executable(test_trees test_trees/test_trees.cpp)

enable_testing()
add_test(NAME trees COMMAND test_trees)
