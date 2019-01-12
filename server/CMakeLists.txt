ADD_DEFINITIONS(-D_APPLE_)
ADD_DEFINITIONS(-std=c++11)

SET(server_src server.cpp server.h)
ADD_EXECUTABLE(server ${server_src})
