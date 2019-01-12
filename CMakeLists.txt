cmake_minimum_required(VERSION 2.8.11)
project(record)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")

ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)
