FROM debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y \
        clang \
        llvm \
        scons

RUN useradd lilith
USER lilith
