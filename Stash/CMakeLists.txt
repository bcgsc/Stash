cmake_minimum_required(VERSION 3.21)
project(Stash)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS "-Xclang -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_STANDARD 14)

file(GLOB BTLLIB_FILES
        include/btllib/*
        )

link_directories(dependencies)
add_library(stash STATIC src/Stash.cpp include/Stash.h src/Read.cpp include/Read.h src/city.cc include/city.h include/config.h ${BTLLIB_FILES})

target_include_directories(stash
        PUBLIC include
        )

target_link_libraries(stash
        btllib
        /usr/local/Cellar/libomp/14.0.0/lib/libomp.dylib
        )

install(TARGETS stash DESTINATION ./)
