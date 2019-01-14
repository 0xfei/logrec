ADD_DEFINITIONS(-D_APPLE_)

ADD_DEFINITIONS(-std=c++11)
link_libraries(pthread)

SET(client_src
        client.cpp client.h
        hashmap.h
        helper.h
        md5/Md5.cpp
        md5/Md5.h
        md5/md5_hashid.h
        md5/time_consuming_md5.cpp
        md5/time_consuming_md5.h
        worker.cpp
        worker.h)
ADD_EXECUTABLE(client ${client_src})

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread")