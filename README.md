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

## Publications

## Authors
