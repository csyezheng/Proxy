cmake_minimum_required(VERSION 3.5)
project(Proxy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp src/net/Server.h src/net/ThreadPool.h src/net/temp.h src/net/threadPool.cpp src/net/server.cpp src/net/Channel.h)
add_executable(Proxy ${SOURCE_FILES})