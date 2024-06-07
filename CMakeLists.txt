cmake_minimum_required(VERSION 3.15)

project(Epam-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
else()
    message(FATAL_ERROR "Unsupported compilation mode")
endif()

message(STATUS "CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})

set(SOURCES
        main.cpp
        threadpool.cpp
        task.cpp
        dataqueue.cpp
)

find_package(Boost COMPONENTS fiber REQUIRED)

add_executable(Epam-cpp ${SOURCES})

target_include_directories(Epam-cpp PRIVATE ${BOOST_INCLUDEDIR})

target_link_libraries(Epam-cpp PRIVATE Boost::boost Boost::fiber)