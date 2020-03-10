cmake_minimum_required(VERSION 3.14)
project(hive)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(
        # kernel headers
        "/usr/local/Cellar/lua/5.3.5_1/include/lua5.3"
        "../luna"
)

add_executable(hive
        hive.cpp
        hive.h
        LICENSE
        main.cpp
        makefile
        README.md
        tools.cpp
        tools.h)
