# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest

ENV PROJECT=TradingPlatform

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install clang

# Install cmake
RUN yes | apt install python3-pip
RUN pip install --upgrade cmake
RUN apt-get install -y git ninja-build vim curl zip unzip tar pkg-config

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/$PROJECT

# Specify the working directory
WORKDIR /usr/src/$PROJECT

RUN rm -rf build
RUN cmake --preset=Ninja-gcc-x64-debug
RUN cd build/Ninja-gcc-x64-debug/
RUN ninja

#CMD ["/bin/bash"]