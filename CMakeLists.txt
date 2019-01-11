cmake_minimum_required(VERSION 3.1.0)
project(logrec)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")

ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)
