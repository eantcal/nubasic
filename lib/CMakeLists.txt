cmake_minimum_required(VERSION 3.5)
project(nuBasicInterpreter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(nuBasicInterpreter STATIC ${SOURCES})
target_compile_features(nuBasicInterpreter PRIVATE cxx_std_17)

# GDI console is always enabled on Windows (non-TINY)
if (WIN32 AND NOT _nubasic_tiny)
  target_link_libraries(nuBasicInterpreter PUBLIC nuWinConsole)
  target_include_directories(nuBasicInterpreter PUBLIC ${CMAKE_SOURCE_DIR}/nuWinConsole)
endif()