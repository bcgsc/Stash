cmake_minimum_required(VERSION 3.21)
project(Demo)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_STANDARD 14)

link_directories(../lib ../Stash/dependencies)
add_executable(target main.cpp)

target_include_directories(target
        PUBLIC ../Stash/include
        )

target_link_libraries(target
        PUBLIC
        stash
        )
