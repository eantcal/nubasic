cmake_minimum_required(VERSION 3.5)
project(nuBasicInterpreter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/parser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ast)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/statements)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/runtime)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/types)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/api)
include_directories(${CMAKE_BINARY_DIR}/generated/include/util)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/util)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(nuBasicInterpreter STATIC ${SOURCES})
target_compile_features(nuBasicInterpreter PRIVATE cxx_std_20)

if(NUBASIC_WITH_LIBFFI AND TARGET unofficial::libffi::libffi)
  target_link_libraries(nuBasicInterpreter PUBLIC unofficial::libffi::libffi)
  target_compile_definitions(nuBasicInterpreter PUBLIC NUBASIC_HAS_LIBFFI=1)
endif()

if(MSVC)
  set_target_properties(nuBasicInterpreter PROPERTIES
    VS_GLOBAL_IntDir "$(SolutionDir)$(ProjectName).dir\\$(Platform)\\$(Configuration)\\"
  )
endif()

# GDI console is always enabled on Windows full builds (not console-only)
if (WIN32 AND NOT _nubasic_console)
  target_link_libraries(nuBasicInterpreter PUBLIC nuWinConsole)
  target_include_directories(nuBasicInterpreter PUBLIC ${CMAKE_SOURCE_DIR}/console/win)
endif()
