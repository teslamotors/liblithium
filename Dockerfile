FROM --platform=linux/amd64 debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y \
        clang \
        gcc-arm-none-eabi \
        gcc-powerpc-linux-gnu \
        llvm \
        mingw-w64 \
        scons
