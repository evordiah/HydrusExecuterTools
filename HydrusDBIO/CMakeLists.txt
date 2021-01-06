cmake_minimum_required(VERSION 3.7)
project(HydrusDBIO)
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Windows)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Linux)
endif()

add_executable(HydrusDBIO main.cpp)
target_include_directories(HydrusDBIO PUBLIC ${CMAKE_SOURCE_DIR}/HydrusFiles
                                      PUBLIC ${CMAKE_SOURCE_DIR}/thirdPartyLib
                                      PUBLIC ${CMAKE_SOURCE_DIR}/thirdPartyLib/pqxx/include)

target_link_directories(HydrusDBIO PUBLIC ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    target_link_libraries(HydrusDBIO  HydrusFiles pqxx tinyxml2)
else()
    target_link_libraries(HydrusDBIO  HydrusFiles pqxx tinyxml2 pthread stdc++fs)
endif()
