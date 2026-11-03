cmake_minimum_required(VERSION 4.3)
project(Awesome_Cool_Team_momentum)

set(CMAKE_CXX_STANDARD 26)

add_executable(Awesome_Cool_Team_momentum main.cpp
        Load_Dataset.cpp
        Load_Dataset.h
        Segment_Tree.cpp
        Segment_Tree.h
        Fenwick_Tree.cpp
        Fenwick_Tree.h)
