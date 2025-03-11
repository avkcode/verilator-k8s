FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    perl \
    python3 \
    make \
    g++ \
    libfl2 \
    libfl-dev \
    zlib1g \
    zlib1g-dev \
    autoconf \
    flex \
    bison \
    help2man \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/verilator/verilator /opt/verilator
WORKDIR /opt/verilator
RUN git checkout stable
RUN autoconf && ./configure && make -j$(nproc) && make install

COPY . /sim
WORKDIR /sim
