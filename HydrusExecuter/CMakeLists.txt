cmake_minimum_required(VERSION 3.7)
project(HydrusExecuter)
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Windows)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Linux)
endif()

file(GLOB CXX_SOURCES *.cpp)

if(WIN32)
    list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_LIST_DIR}/linuxprocess.cpp")
else()
    list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_LIST_DIR}/winprocess.cpp")
endif()

add_executable(HydrusExecuter ${CXX_SOURCES})

target_include_directories(HydrusExecuter PUBLIC ${CMAKE_SOURCE_DIR}/HydrusFiles
                                          PUBLIC ${CMAKE_SOURCE_DIR}/thirdPartyLib
                                          PUBLIC ${CMAKE_SOURCE_DIR}/thirdPartyLib/pqxx/include)

target_link_directories(HydrusExecuter PUBLIC ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    target_link_libraries(HydrusExecuter  HydrusFiles pqxx tinyxml2 b64)
else()
    target_link_libraries(HydrusExecuter  HydrusFiles pqxx tinyxml2 b64 pthread stdc++fs)
endif()