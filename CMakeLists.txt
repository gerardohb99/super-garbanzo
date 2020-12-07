cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -O3 -DNDEBUG -std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors")

add_executable(untitled main.cpp)