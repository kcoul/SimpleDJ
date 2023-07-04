# Build and run:
# * build the container:
# #   docker build -t clion/ubuntu/cpp-env:1.0 -f Dockerfile .
#
# Troubleshooting commands:
# * if you need an interactive docker shell run:
# #   docker run --rm -it clion/ubuntu/cpp-env:1.0 /bin/sh
# * and if you don't want to use clion, you can mount the volume using:
# #   docker run -rm -it -v $(pwd):/app clion/ubuntu/cpp-env:1.0

# base image
FROM ubuntu:20.04

# docker defaults to /tmp, clion defaults to /tmp/misc so we avoid that future problem entirely
WORKDIR /app

#TODO define target-specific env variables

# set permissions
RUN chmod 0755 -R /app

# non interactive mode plays nice with CLion debug window
RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

# setup
RUN apt-get update \
  && apt-get install -y build-essential \
      gcc \
      g++ \
      gdb \
      git \
      clang \
      make \
      ninja-build \
      cmake \
      pkg-config \
      libfreetype6-dev \
      libasound2-dev \
      libwebkit2gtk-4.0-dev \
      libgtk-3-dev \
      libcurl4-openssl-dev \
      autoconf \
      automake \
      libtool \
      valgrind \
      locales-all \
      dos2unix \
      rsync \
      tar \
      python \
      python-dev \
  && apt-get clean
