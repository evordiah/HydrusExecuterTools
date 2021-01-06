
cmake_minimum_required(VERSION 3.7)
project(tinyxml2)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_library(tinyxml2 STATIC tinyxml2.cpp)
