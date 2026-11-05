
cmake_minimum_required(VERSION 3.10)

project(Awesome_Cool_Team_momentum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Awesome_Cool_Team_momentum
    main.cpp
    Fenwick_Tree.cpp
    Segment_Tree.cpp
    Load_Dataset.cpp
)
