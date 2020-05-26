# Download base image
FROM ubuntu:18.04

WORKDIR /usr/local/src

# Copy the code
RUN mkdir ./rdfev/
COPY ./src ./rdfev/src
COPY ./include ./rdfev/include
COPY ./meson.build ./rdfev/

# Install dependencies
RUN apt-get update && apt-get -qq install \
    pkg-config \
    g++ \
    cmake \
    python3 \
    python3-pip \
    git \
    liblz4-dev \
    libzstd-dev \
    libkyotocabinet16v5 \
    libkyotocabinet-dev

# Get and build RocksDB 
RUN mkdir ./tmp/ \
    && git clone https://github.com/facebook/rocksdb/ ./tmp/rocksdb/
WORKDIR /usr/local/src/tmp/rocksdb/
RUN git checkout v6.6.4 \
    && mkdir ./cstbuild
WORKDIR /usr/local/src/tmp/rocksdb/cstbuild/
RUN cmake /usr/local/src/tmp/rocksdb/ -DCMAKE_BUILD_TYPE=Release -DWITH_LZ4=1 -DWITH_ZSTD=1 -DWITH_TOOLS=0 -DWITH_TESTS=0 -DWITH_RUNTIME_DEBUG=0 \
    && make -j 2
WORKDIR /usr/local/src/
RUN mkdir -p ./rdfev/lib/ \
    && cp /usr/local/src/tmp/rocksdb/cstbuild/librocksdb.a /usr/local/src/rdfev/lib/ \
    && cp -r /usr/local/src/tmp/rocksdb/include/rocksdb /usr/local/src/rdfev/include/ \
    && rm -rf /usr/local/src/tmp/

# Install python libs
RUN pip3 install meson ninja scipy matplotlib

# Build
WORKDIR /usr/local/src/rdfev/
RUN meson build/ --buildtype=release
WORKDIR /usr/local/src/rdfev/build/
RUN ninja

# Add binaries to PATH
WORKDIR /usr/local/src/rdfev/
RUN mkdir -p ./bin/ \
    && mv ./build/src/lowdiff ./bin/ \
    && mv ./build/src/highdiff ./bin/ \
    && mv ./build/src/memhighdiff ./bin/ \
    && mkdir -p /python_scripts/ \
    && cp ./src/Utils/* /python_scripts/
ENV PATH /usr/local/src/rdfev/bin/:$PATH
RUN rm -rf ./build

WORKDIR /

# Setup commands
CMD ["/bin/bash"]