cmake_minimum_required(VERSION 3.5)
project(nuBasicIDE)

if(NOT WIN32)
  message(WARNING "Windows IDE can only be built on Windows")
  return()
endif()

# Source files
set(IDE_SOURCES
  nuBasicEditor.cc
  stdafx.cpp
  textinfobox.cc
  tab_container.cc
  toolbar.cc
)

# Header files
set(IDE_HEADERS
  nuBasicEditor.h
  Resource.h
  stdafx.h
  targetver.h
  textinfobox.h
  tab_container.h
  toolbar.h
  ${CMAKE_SOURCE_DIR}/include/stb_image.h
)

# Resource files
set(IDE_RESOURCES
  nuBasicEditor.rc
)

# Create the executable
add_executable(nuBasicIDE WIN32 ${IDE_SOURCES} ${IDE_HEADERS} ${IDE_RESOURCES})
set_target_properties(nuBasicIDE PROPERTIES OUTPUT_NAME "NuBasicIDE")

# Set C++ standard
target_compile_features(nuBasicIDE PRIVATE cxx_std_17)

# Include directories
# SCINTILLA_INCLUDE_DIRS and LEXILLA_INCLUDE_DIRS are set by cmake/ScintillaFetch.cmake
target_include_directories(nuBasicIDE PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${SCINTILLA_INCLUDE_DIRS}
  ${LEXILLA_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/nuWinConsole
)

# Preprocessor definitions
target_compile_definitions(nuBasicIDE PRIVATE
  _CRT_SECURE_NO_WARNINGS
  WIN32
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  _WINDOWS
)

# Compiler options
if(MSVC)
  target_compile_options(nuBasicIDE PRIVATE
    /W3
    $<$<CONFIG:Release>:/O2 /Ob2>
    $<$<CONFIG:Debug>:/Od /Zi>
  )
  # Use static runtime library
  set_property(TARGET nuBasicIDE PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Ensure SciLexer.dll is built before the IDE (when using fetched version)
# and copy it next to nuBasicIDE.exe so it can be found at runtime.
if(TARGET SciLexer)
  add_dependencies(nuBasicIDE SciLexer)
  add_custom_command(TARGET nuBasicIDE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:SciLexer>
      $<TARGET_FILE_DIR:nuBasicIDE>
    COMMENT "Copying SciLexer.dll next to nuBasicIDE.exe"
  )
endif()

# Link against interpreter library
target_link_libraries(nuBasicIDE PRIVATE
  nuBasicInterpreter
  Comctl32.lib
  kernel32.lib
  user32.lib
  gdi32.lib
  winspool.lib
  comdlg32.lib
  advapi32.lib
  shell32.lib
  ole32.lib
  oleaut32.lib
  uuid.lib
  odbc32.lib
  odbccp32.lib
)

target_link_libraries(nuBasicIDE PRIVATE nuWinConsole)

# Install target
install(TARGETS nuBasicIDE DESTINATION bin)

if(TARGET SciLexer)
  install(TARGETS SciLexer RUNTIME DESTINATION bin)
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples/"
  DESTINATION examples
  FILES_MATCHING
  PATTERN "*.bas"
  PATTERN "manifest.txt")


