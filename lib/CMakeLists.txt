cmake_minimum_required(VERSION 2.8.12)
project(nubasicinterpreter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14" )
add_library(nubasicinterpreter STATIC ${SOURCES})
