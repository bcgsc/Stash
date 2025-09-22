project(Stash LANGUAGES CXX)

# Library sources and headers
add_library(Stash
    Source/Stash.cpp
    Include/Stash/Stash.h

    Source/Sequence.cpp
    Include/Stash/Sequence.h

    Source/FastaReader.cpp
    Include/Stash/FastaReader.h

    Source/CityHash/city.cc
    Include/CityHash/city.h
    Include/CityHash/config.h
)

# Public headers
target_include_directories(Stash
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        ${conda_env_path}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# OpenMP
find_package(OpenMP REQUIRED)

# Library linkage
target_link_directories(Stash
    PUBLIC
        ${conda_env_path}/lib
)

target_link_libraries(Stash
    PRIVATE
        libbtllib.a
    PUBLIC
        OpenMP::OpenMP_CXX
)
