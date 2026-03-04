FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN dpkg --add-architecture arm64 && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      cmake \
      build-essential \
      pkg-config \
      gcc-aarch64-linux-gnu \
      g++-aarch64-linux-gnu \
      binutils-aarch64-linux-gnu \
      libglfw3-dev \
      libcjson-dev \
      libstb-dev \
      libglfw3-dev:arm64 \
      libcjson-dev:arm64 \
      libstb-dev:arm64 \
      zip \
      unzip && \
    rm -rf /var/lib/apt/lists/*
