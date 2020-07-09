# RDFev

Tool to compare RDF files in the N-Triples format.  

## Requirements

Here are the various requirements in order to use this tool.   
RDFev has only been tested on Linux systems.

### Runtime Dependencies
* [KyotoCabinet](https://fallabs.com/kyotocabinet) (it is dynamically linked : the .so is needed in the system)
* OpenMP runtime library

### Compile Dependencies
* C++ 11 compatible compiler (GCC 5 or newer recommended)
* [KyotoCabinet](https://fallabs.com/kyotocabinet) developement headers
* OpenMP developement headers
* [Meson build system](https://mesonbuild.com/)
* [LZ4](https://lz4.github.io/lz4/) headers and library
* [RocksDb](https://rocksdb.org/) static library. Compile it for your system with support for LZ4 and ZSTD and put librocksdb.a in the ./lib/ folder. Put the rocksdb/include/ from rocksdb repos into the ./include/ folder.

### Python Requirements
Python 3.6+ is recommended.  

## Compile

First create the build directory with **Meson** :
```bash
meson path/to/builddir --buildtype=release
```
Use **ninja** to compile the project :
```bash
cd path/to/builddir
ninja
```
Three executable are produced : *lowdiff* for low level diffs, and *highdiff* and *memhighdiff* for high level diff.  
The regular *highdiff* put everything on a database, so it has a relatively low memory footprint, *memhighdiff* do more in memory and should be faster.
The executables are located in *path/to/builddir/src/*

## Usage

The tool should be relatively straight forward to use.  
Usage template :

```bash
./lowdiff file1 file2 database_mode result_name
./highdiff file1 file2 result_name
```
### Parameters :
* *file1* and *file2* : the files to compare (oldest file first, newest file second)
* *database_mode* : the kind of database used by the tool. Options are:
    * *onmemory* : on memory database (std::unordered_map) (fast)
    * *kcondisk* : on disk hash database from the KyotoCabinet libray (slow)
    * *multikcondisk* : multiple on disk hash database (slightly faster than kcondisk for big files)
    * *rocksdb* : on disk database from the [RocksDB](https://rocksdb.org/) library. (should be faster than KyotoCabinet for large databases, and support compression via LZ4)
    * *hybrid* : on memory database for triples IDs, on disk database (rocksdb) for triples
* *result_name* : the name that will be used by the tool to create files (on disk databases and result file) 

### Output

The low level diff (lowdiff) will output those numbers in the output file, in order, on the same line, separated with a white space:
* number of additions
* number of deletions
* number of triples in file 2
* number of triples in file 1
* percentage of additions
* percentage of deletions
* size of the union between the two files

The high level diff (highdiff and memhighdiff) will output thos numbers in a similar fashion:
* number of triples in file 1
* number of triples in file 2
* number of entities change
* number of entity additions
* number of entity deletions
* number of object updates
* number of object additions
* number of object deletions
* number of added components
* number of deleted components
* the average number of triples affected by an entity change
* the total number of components (file1 + file2)

## Docker

A docker file is provided to ease the use of the tool. Thanks to Docker, RDFev can also be run on Windows.  

### Build
After having cloned this repository, run this command within RDFev's folder:  
```bash
docker build . -t rdfev
```
You may have to use *"sudo"* on Linux.

### Docker usage

The docker image is made to be used in interactive mode.
You can run the image by using a command similar to this one:
```bash
docker run -it --rm -v "/path/to/datasets/":"/datasets" image_id
```
This command will open a bash terminal within the docker container. You may have to use *"sudo"* on Linux.  
The *"-v"* option is used to mount a folder in the docker container, use this to make your RDF data available.  
Then, you can use any commands like shown earlier in this readme, they are already added to the system path.  
The Python scripts are located in *"/python_scripts/"* if you wish to use them.