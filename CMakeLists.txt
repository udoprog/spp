cmake_minimum_required(VERSION 2.6)
project(spp)

find_package(Boost COMPONENTS thread filesystem system signals REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

include_directories("${CMAKE_SOURCE_DIR}/src/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(spp_LIBRARIES ${Boost_LIBRARIES})

add_subdirectory(src)
