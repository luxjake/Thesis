# RDF Diff

Tool to compare RDF files.  

## Requirements

Here are the various requirements in order to use this tool.

### Runtime Dependencies
* [KyotoCabinet](https://fallabs.com/kyotocabinet) (it is dynamically linked : the .so is neened in the system)
* OpenMP runtime library

### Compile Dependencies
* C++ 11 compatible compiler (GCC 5 or newer recommended)
* [KyotoCabinet](https://fallabs.com/kyotocabinet) developement headers
* OpenMP developement headers
* [Meson build system](https://mesonbuild.com/)
* [LZ4](https://lz4.github.io/lz4/) headers and library
* [RocksDb](https://rocksdb.org/) static library. Compile it for your system with support for LZ4 and ZSTD and put librocksdb.a in the ./lib/ folder.

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
* *database_mode* : the kind of database used by the tool. Options are :
    * *onmemory* : on memory database (std::unordered_map) (fast)
    * *kcondisk* : on disk hash database from the KyotoCabinet libray (slow)
    * *multikcondisk* : multiple on disk hash database (slightly faster than kcondisk for big files)
    * *rocksdb* : on disk database from the [RocksDB](https://rocksdb.org/) library. (should be faster than KyotoCabinet for large databases, and support compression via LZ4)
    * *hybrid* : on memory database for triples IDs, on disk database (rocksdb) for triples
* *result_name* : the name that will be used by the tool to create files (on disk databases and result file) 
