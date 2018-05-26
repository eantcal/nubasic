cmake_minimum_required(VERSION 2.8.12)
project(mipjson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../miptknzr/include)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14" )
add_library(mipjson STATIC ${SOURCES})
target_link_libraries(mipjson PUBLIC miptknzr)
