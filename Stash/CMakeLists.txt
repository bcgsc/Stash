project(Stash)

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

target_include_directories(Stash
	PRIVATE
		Source
	PUBLIC
		Include
)

find_package(OpenMP REQUIRED)

target_link_libraries(Stash
	PRIVATE
		libbtllib.a
	PUBLIC
		OpenMP::OpenMP_CXX
)
