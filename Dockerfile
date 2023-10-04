# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest

ENV PROJECT=TradingPlatform

RUN apt-get -y update && apt-get install -y
RUN apt-get install -y g++
RUN apt-get install -y gcc
RUN apt-get install -y gdb
RUN apt-get install -y clang
RUN apt-get install -y ninja-build

RUN apt-get install -y git
RUN apt-get install -y curl
RUN apt-get install -y zip
RUN apt-get install -y unzip
RUN apt-get install -y tar
RUN apt-get install -y pkg-config

RUN yes | apt install python3-pip
RUN pip install --upgrade cmake
RUN export PATH=/usr/local/bin/:$PATH

#ccache
RUN apt install -y ccache
RUN /usr/sbin/update-ccache-symlinks
RUN export PATH=/usr/lib/ccache:$PATH

RUN git clone https://github.com/Microsoft/vcpkg.git
RUN vcpkg/bootstrap-vcpkg.sh

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /tmp/$PROJECT

# Specify the working directory
WORKDIR /tmp/$PROJECT

RUN cd ..
RUN git clone https://github.com/Microsoft/vcpkg.git
RUN vcpkg/bootstrap-vcpkg.sh

RUN cd /tmp/$PROJECT
RUN rm -rf build
#RUN cmake --preset=Ninja-gcc-x64-debug
#RUN cd build/Ninja-gcc-x64-debug/
#RUN ninja

#CMD ["/bin/bash"]