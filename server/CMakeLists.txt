ADD_DEFINITIONS(-D_APPLE_)
ADD_DEFINITIONS(-std=c++11)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread")

SET(server_src server.cpp server.h)
ADD_EXECUTABLE(server ${server_src})
