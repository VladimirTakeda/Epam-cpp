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

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS system thread program_options)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost")
endif ()

include_directories(${Boost_INCLUDE_DIRS})

set(SOURCES
        main.cpp
        src/task.cpp
        src/dataqueue/sharedbuffer.cpp
        src/threadpool.cpp
        src/util.cpp
        src/dataqueue/interthreaddataqueue.cpp
        src/dataqueue/interthreaddataqueue.h
)

add_executable(Epam-cpp ${SOURCES})

target_link_libraries(Epam-cpp PRIVATE Boost::boost Boost::system Boost::thread Boost::program_options)