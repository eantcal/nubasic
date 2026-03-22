cmake_minimum_required(VERSION 3.5)
project(nubasicinterpreter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(nubasicinterpreter STATIC ${SOURCES})
target_compile_features(nubasicinterpreter PRIVATE cxx_std_17)

# GDI console is always enabled on Windows (non-TINY)
if (WIN32 AND NOT _nubasic_tiny)
  target_link_libraries(nubasicinterpreter PUBLIC nuwinconsole)
  target_include_directories(nubasicinterpreter PUBLIC ${CMAKE_SOURCE_DIR}/nuwinconsole)
endif()