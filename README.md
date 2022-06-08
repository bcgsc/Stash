# Stash

This repository contains the implementation for the Stash data structure

## Dependencies
  * Compiler with OpenMP Support
  * [meson](https://mesonbuild.com/)
  * [ninja](https://ninja-build.org/)
  * [btllib](https://github.com/bcgsc/btllib) v1.4.3+

The dependencies can be installed through [Conda](https://docs.conda.io/en/latest/) package manager:
```
conda install -c conda-forge meson ninja 
conda install -c bioconda btllib
```

## Installation

To build the Stash library and a simple demo of its application, run the following commands from within the `Stochastic_Tile_Hashing` directory:
```
meson setup build --buildtype release
cd build
meson install
```

The Stash library is installed at `Stochastic_Tile_Hashing/subprojects/Stash/lib`, and the demo `Stash_Demo` is avaliable at `Stochastic_Tile_Hashing/build`.

## Usage

Stash provides its functionalities through a simple interface.

### Stash Reads

Each Stash read has a unique ID assigned to it. A read can be created using the `Read(char* sequence, int length)` constructor in `Read.h`, and it can be assigned an ID by manually calling the `void hashReadID(int ID, int B1, int B2)` function, where `B1` and `B2` are the parameters of the Stash. If not manually called, the function is automatically called if the read is used in filling the Stash.

### Stash

A Stash can be created using the `Stash(uint64_t rows, std::vector<std::string> & spacedSeeds, int T1, int T2)` constructor.
* `rows`: Determines the number of rows of the Stash.
* `spacedSeeds`: A vector of strings, where each string represents a spaced seed frame.
* `T1`: The number of bits used to represent a column in Stash.
* `T2`: The number of bits used to represent a tile value in Stash.

Calling `void save(const char* path)` on a Stash stores its data and all parameters into a binary file. The Stash can be later restored using the `Stash(const char* path)` constructor.

### Filling the Stash

The `void fill(std::vector <Read*> & reads, int threads=-1)` function can be used to fill a vector of reads into the Stash, where `threads` is the number of OpenMP threads used to perform the function (by default, `omp_get_max_threads()` is used). Each read will be assigned a unique ID if it is unassigned. `void print()` can be used to display the content of the Stash for debug purposes.

### Tiles

Direct access to Stash tiles is available through the following functions.
* `void writeTile(uint64_t row, int column, int value);`
* `int readTile(uint64_t row, int column);`

## Publications

## Authors
