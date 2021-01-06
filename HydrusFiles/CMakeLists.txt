cmake_minimum_required(VERSION 3.7)
project(HydrusFiles)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
file(GLOB CXX_SOURCES *.cpp)
add_library(HydrusFiles STATIC ${CXX_SOURCES})
target_include_directories(HydrusFiles PUBLIC ${CMAKE_CURRENT_LIST_DIR}
                                       PUBLIC ${CMAKE_SOURCE_DIR}/thirdPartyLib
                                       PUBLIC ${CMAKE_SOURCE_DIR}/thirdPartyLib/pqxx/include)

