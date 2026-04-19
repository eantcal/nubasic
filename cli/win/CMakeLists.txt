cmake_minimum_required(VERSION 3.5)
project(nuBasicCLI)

if(NOT WIN32)
  message(WARNING "nuBasicCLI can only be built on Windows")
  return()
endif()

set(NUBASICGDI_SOURCES
  nuBasicCLI.cpp
)

set(NUBASICGDI_HEADERS
  framework.h
  Resource.h
  targetver.h
  nuBasicCLI.h
)

set(NUBASICGDI_RESOURCES
  nuBasicCLI.rc
)

# GDI/graphics subsystem executable (nubasicgdi.exe).
# Opens its own console window; used for interactive GUI-style sessions.
add_executable(nuBasicGDI WIN32
  ${NUBASICGDI_SOURCES}
  ${NUBASICGDI_HEADERS}
  ${NUBASICGDI_RESOURCES})

set_target_properties(nuBasicGDI PROPERTIES OUTPUT_NAME "nubasicgdi")

target_compile_features(nuBasicGDI PRIVATE cxx_std_20)

target_include_directories(nuBasicGDI PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/api
  ${CMAKE_SOURCE_DIR}/include/ast
  ${CMAKE_SOURCE_DIR}/include/parser
  ${CMAKE_SOURCE_DIR}/include/runtime
  ${CMAKE_SOURCE_DIR}/include/statements
  ${CMAKE_SOURCE_DIR}/include/types
  ${CMAKE_SOURCE_DIR}/include/util
  ${CMAKE_SOURCE_DIR}/console/win
  .
)

target_compile_definitions(nuBasicGDI PRIVATE
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  _WINDOWS
)

if(MSVC)
  target_compile_options(nuBasicGDI PRIVATE
    /W3
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Debug>:/Od /Zi>
  )
  set_property(TARGET nuBasicGDI PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(nuBasicGDI PRIVATE
  nuBasicInterpreter
  nuWinConsole
)

install(TARGETS nuBasicGDI DESTINATION bin COMPONENT Core)

# Console-subsystem text-mode CLI (nubasic.exe).
# Inherits the invoking terminal — stdio goes to the calling process, shell
# redirections (> file, pipes) work correctly.  No GDI window is created.
add_executable(nuBasicCLI
  ${CMAKE_SOURCE_DIR}/cli/main.cc
)

set_target_properties(nuBasicCLI PROPERTIES OUTPUT_NAME "nubasic")
target_compile_features(nuBasicCLI PRIVATE cxx_std_20)

target_include_directories(nuBasicCLI PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/api
  ${CMAKE_SOURCE_DIR}/include/ast
  ${CMAKE_SOURCE_DIR}/include/parser
  ${CMAKE_SOURCE_DIR}/include/runtime
  ${CMAKE_SOURCE_DIR}/include/statements
  ${CMAKE_SOURCE_DIR}/include/types
  ${CMAKE_SOURCE_DIR}/include/util
  ${CMAKE_SOURCE_DIR}/console/win
  .
)

target_compile_definitions(nuBasicCLI PRIVATE
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
  target_compile_options(nuBasicCLI PRIVATE
    /W3
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Debug>:/Od /Zi>
  )
  set_property(TARGET nuBasicCLI PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(nuBasicCLI PRIVATE
  nuBasicInterpreter
  nuWinConsole
)

install(TARGETS nuBasicCLI DESTINATION bin COMPONENT Core)
