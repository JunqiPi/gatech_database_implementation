cmake_minimum_required(VERSION 3.10)
project(BuzzDB_Lab1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

add_library(student_libs
    src/include/user/user.cpp
    src/include/post/post.cpp
    src/include/engagement/engagement.cpp
)

target_include_directories(student_libs PUBLIC
    src/include/user
    src/include/post
    src/include/engagement
)

add_executable(lab1.out test/test.cpp)
target_include_directories(lab1.out PRIVATE
    src/include/user
    src/include/post
    src/include/engagement
    src/flat_file
)
target_link_libraries(lab1.out PRIVATE student_libs)
