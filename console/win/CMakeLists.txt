cmake_minimum_required(VERSION 3.5)
project(nuWinConsole)

if(NOT WIN32)
  message(WARNING "nuWinConsole can only be built on Windows")
  return()
endif()

# ===== Static Library for nuBASIC integration =====

# Source files for library
set(LIB_SOURCES
  ConsoleWindow.cpp
  ConsoleBuffer.cpp
  nu_winconsole_api.cpp
)

# Header files for library
set(LIB_HEADERS
  ConsoleWindow.h
  ConsoleBuffer.h
  ConsoleConfig.h
  nu_winconsole_api.h
)

# Create static library
add_library(nuWinConsole STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Set C++ standard for library
target_compile_features(nuWinConsole PUBLIC cxx_std_20)

# Include directories for library (PUBLIC so consumers can use them)
target_include_directories(nuWinConsole PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

# Preprocessor definitions for library
# UNICODE/_UNICODE are PRIVATE: nuWinConsole uses wide-char Win32 APIs internally
# but must not force Unicode mode on consumers (interpreter / IDE use MBCS/ANSI).
target_compile_definitions(nuWinConsole
  PUBLIC
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _WINDOWS
  PRIVATE
    UNICODE
    _UNICODE
)

# Compiler options for library
if(MSVC)
  target_compile_options(nuWinConsole PUBLIC
    /W3
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Debug>:/Od /Zi>
  )
  # Use static runtime library
  set_property(TARGET nuWinConsole PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Link libraries for the static library
target_link_libraries(nuWinConsole PUBLIC
  Gdi32
  User32
  kernel32
)


