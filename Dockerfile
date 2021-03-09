FROM ubuntu:18.04

# prevent tzdata package from asking for time zone (indirect dependency of new python versions)
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y software-properties-common \
    && add-apt-repository ppa:deadsnakes/ppa

RUN apt-get update \
    && apt-get install -y \
        clang \
        gcc-arm-none-eabi \
        gcc-multilib \
        git \
        llvm \
        locales \
        mingw-w64 \
        python3-pip \
        python3.6-dev \
        python3.7-dev \
        python3.8-dev \
        python3.9-dev \
        python3.9-distutils \
        scons

RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

RUN python3 -m pip install --upgrade pip
RUN python3 -m pip install virtualenv

WORKDIR /src
