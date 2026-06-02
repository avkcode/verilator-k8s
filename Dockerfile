FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    verilator \
    && rm -rf /var/lib/apt/lists/*

COPY . /sim
WORKDIR /sim
