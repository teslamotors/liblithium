FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y \
        clang \
        gcc-arm-none-eabi \
        llvm \
        mingw-w64 \
        scons
