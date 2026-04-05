cmake_minimum_required(VERSION 3.5)
project(nuBasicCLI)

if(NOT WIN32)
  message(WARNING "nuBasicCLI can only be built on Windows")
  return()
endif()

set(NUBASICCLI_SOURCES
  nuBasicCLI.cpp
)

set(NUBASICCLI_HEADERS
  framework.h
  Resource.h
  targetver.h
  nuBasicCLI.h
)

set(NUBASICCLI_RESOURCES
  nuBasicCLI.rc
)

add_executable(nuBasicCLI WIN32
  ${NUBASICCLI_SOURCES}
  ${NUBASICCLI_HEADERS}
  ${NUBASICCLI_RESOURCES})

# Keep executable name `nubasic.exe` (IDE and scripts invoke `nubasic`).
set_target_properties(nuBasicCLI PROPERTIES OUTPUT_NAME "nubasic")

target_compile_features(nuBasicCLI PRIVATE cxx_std_20)

target_include_directories(nuBasicCLI PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/console/win
  .
)

target_compile_definitions(nuBasicCLI PRIVATE
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  _WINDOWS
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

install(TARGETS nuBasicCLI DESTINATION bin)
