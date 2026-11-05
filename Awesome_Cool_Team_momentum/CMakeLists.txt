
cmake_minimum_required(VERSION 3.10)
project(SoccerMomentumTracker CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(include)
add_executable(momentum_tracker src/main.cpp src/Tracker.cpp src/Momentum.cpp)
add_executable(test_trees tests/test_trees.cpp src/Momentum.cpp)

enable_testing()
add_test(NAME trees COMMAND test_trees)
