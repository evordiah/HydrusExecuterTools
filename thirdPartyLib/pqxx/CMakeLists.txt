cmake_minimum_required(VERSION 3.7)
project(pqxx)
if(NOT PostgreSQL_FOUND)
    if(POLICY CMP0074)
        cmake_policy(PUSH)
        cmake_policy(SET CMP0074 NEW)
    endif()

    find_package(PostgreSQL REQUIRED)

    if(POLICY CMP0074)
        cmake_policy(POP)
    endif()
endif()

file(GLOB CXX_SOURCES src/*.cxx)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_library(pqxx STATIC ${CXX_SOURCES})
target_include_directories(pqxx 
                           PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
                           PRIVATE ${PostgreSQL_INCLUDE_DIRS})  

target_link_libraries(pqxx PRIVATE ${PostgreSQL_LIBRARIES})
if(WIN32)
    target_link_libraries(pqxx PUBLIC wsock32 ws2_32)
endif()
