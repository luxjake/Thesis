project('RDF Diff', 'cpp', version : '2.0', default_options : ['cpp_std=c++11'])

compiler = meson.get_compiler('cpp')

# Include directory, for all shared headers
incdir = [include_directories('include')]

# Library path - add libraries there
lib_path = meson.source_root() + '/lib'

# Kyoto Cabinet dependency
kcdep = dependency('kyotocabinet')

# RocksDB dependency
# RocksDB sould be built separately
# librocksdb.a should be in ./lib and RocksDB headers should be in ./include
rockdep = compiler.find_library('librocksdb', dirs : lib_path)

# RocksDB Compression lib dependencies
lz4dep = dependency('liblz4')
lzdep = dependency('zlib')
zstddep = dependency('libzstd')

# OpenMP dependency
ompdep = dependency('openmp')

deps = [kcdep, ompdep, rockdep, lz4dep, lzdep, zstddep]

subdir('src')
