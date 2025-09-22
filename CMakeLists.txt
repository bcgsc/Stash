cmake_minimum_required(VERSION 3.10)

# Project metadata before adding subdirectories
project(StashEXE LANGUAGES CXX)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build (Debug or Release)" FORCE)
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
add_compile_options(-Wall)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra") # Uncomment if needed

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Add the library
add_subdirectory(Stash)

# Main executable
add_executable(StashEXE main.cpp)

# Link executable with Stash library
target_link_libraries(StashEXE PRIVATE Stash)

# Set output name/path
set_target_properties(StashEXE PROPERTIES
    OUTPUT_NAME "Stash"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Stash"
)
