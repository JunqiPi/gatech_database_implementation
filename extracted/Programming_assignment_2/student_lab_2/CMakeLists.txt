cmake_minimum_required(VERSION 3.10)
project(BuzzDB_Lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -Wall -Werror -Wextra -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Werror -Wextra -pthread")

add_library(student_libs
    src/buffer/buffer_manager.cpp
    src/include/storage/storage_manager.cpp
    src/include/buffer/frame_lock_table.cpp
)

target_include_directories(student_libs PUBLIC
    src/include
)

add_executable(lab2.out test/test.cpp)
target_include_directories(lab2.out PRIVATE
    src/include
)
target_link_libraries(lab2.out PRIVATE student_libs)
