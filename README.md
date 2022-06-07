### Description
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
