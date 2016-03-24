cmake_minimum_required(VERSION 3.0)

project(gtl)

set(GTL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include_directories(BEFORE ${GTL_DIR})
# include_directories(${BOOST_DIR})

file(GLOB SOURCES *.cpp)
