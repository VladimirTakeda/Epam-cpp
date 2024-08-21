cmake_minimum_required(VERSION 3.15)

project(Epam-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
else ()
    message(FATAL_ERROR "Unsupported compilation mode")
endif ()

message(STATUS "CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})

set(SOURCES
        main.cpp
        src/task.cpp
        src/dataqueue/sharedbuffer.cpp
        src/sharedmemorymanager.cpp
        src/threadpool.cpp
        src/utils/util.cpp
        src/dataqueue/interprocessdataqueue.cpp
        src/dataqueue/interprocessdataqueue.h
        src/dataqueue/interthreaddataqueue.cpp
        src/dataqueue/interthreaddataqueue.h
        src/utils/logger.cpp
        src/utils/logger.h
        src/utils/garbadgecollector.cpp
        src/utils/garbadgecollector.h
)

add_executable(Epam-cpp ${SOURCES})