cmake_minimum_required(VERSION 3.19)
project(hw1-os)

set(CMAKE_CXX_STANDARD 14)

add_executable(hw1-os smash.cpp Commands.cpp signals.cpp)