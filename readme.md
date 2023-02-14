# Cannons Algorithm in DARTS using Dataflow Software Pipelining

Cannon's Algorithm is a distributed algorithm for matrix multiplication of two-dimensional meshes, first described in 1969 by Lynn Elliot Cannon. This repository contains its implementation using DARTS runtime using and without using Dataflow Software Pipelining techniques. 

## Prerequisites

Please follow the instructions on the respective dependency website to install them. 

* cmake : [https://cmake.org/](https://cmake.org/)
* HWLOC : [http://www.open-mpi.org/projects/hwloc/](http://www.open-mpi.org/projects/hwloc/)
* Intel Thread Building Blocks (Optional) : [https://github.com/oneapi-src/oneTBB](https://github.com/oneapi-src/oneTBB)
* Pthreads.


## Steps to Install

* Clone cannon-dfswp repository.
  `$ git clone https://github.com/sraskar/cannon-dfswp.git`

* Enter the directory and Create a build directory.
  ```bash
  $ cd cannon-dfswp
  $ mkdir build 
  $ cd build 
  ```

* Use `cmake` to configure the repository. Please make sure to add relevant paths for MKL and HWLOC in the camke command using `-DMKL_ROOT_DIR=/path/to/mkl_root` and `-DHWLOC_ROOT=/path/to/hwloc_root` flags. 
  ```bash
  $ CC=gcc CXX=g++ cmake -DCBLAS=OFF -DMKL=ON -DMKL_ROOT_DIR=/path/to/mkl_root -DHWLOC_ROOT=/path/to/hwloc_root ../
  ```

* Use `make` to compile code. 
  ```bash
  $ make 
  ``` 

### `cmake` Options 

* Debugging Option 
Debug ie. compile with `-g` : `-DDEBUG=ON` 
By default cmake compiles in release mode ie. compiles with -O3

* Install directory: `-DCMAKE_INSTALL_PREFIX=/path/to/directory/`
  By default, Darts will install in bash variable HOME/darts

* `DMKL=ON -DMKL_ROOT_DIR=/path/to/mkl_root`
  This option will enable execution with MKL version on the specified path.  

* `cmake` cannot find a `hwloc/TBB` 
  First, make sure you have hwloc/TBB installed on your system. Second cmake will look for hwloc/TBB in specific directories. To find if you installation is located in on of these directories, navigate in your source to the cmake/Modules directory and view the FindHwloc.cmake/FindTBB.cmake file. You may add your search path to these files rather than moving your installation.

## Running Cannons Algorithm

* The source code for Cannons Algorithm is Present in the directory `apps/Cannon`
* After `make` command, code is built in the directory `build/apps/Cannon`
* The code needs to be executed with the required command line arguments. 
`$ ./CannonTP_test numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile` where 
  * `numSU`: Number of Synchronization Units
  * `numCU`: Number of Compute Units 
  * `Matsize`: Size of Square Matrix
  * `sqrtNumTiles`: Square root of the total number of tiles for the matrix 
  * `SqrtNumElementPerTile`: Square root of Number of elements per Tile for matrix. 
* For example, `./cannonTP_test 1 1 64 2 32` 
* `mmCannonTime.sh` script contains some configurations. 

## Running Cannons Algorithm with Dataflow Software Pipelining 

* The source code for Cannons Algorithm with dataflow Software Pipelining is Present in the directory `apps/Cannon-dfswp`
* After `make` command, code is built in the directory `build/apps/Cannon-dfswp`
* * The code needs to be executed with the required command line arguments. 
`$ ./Cannon_dfswp numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile`. The meaning of these command line arguments is the same as the one mentioned above. 
* `mmCannonTime.sh` script contains some configurations. 
