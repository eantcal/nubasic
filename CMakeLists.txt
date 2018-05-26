cmake_minimum_required(VERSION 2.8.12)

if (NOT APPLE)
  option(WITH_X11 "X11 graphics/sound" ON)
  option(WITH_IDE "GTK+2 Integrated Development Environment (IDE)" ON)
else()
  message("On this platform we don't suport X11 and GTK+2 IDE")
endif()

if (WITH_X11)
  include (${CMAKE_ROOT}/Modules/FindX11.cmake)
  if(X11_FOUND)
    add_definitions(-DCMAKE_HAS_X)

    find_program(xterm_prog NAMES xterm)
    if (NOT xterm_prog)
      message("Warning: could not find xterm")
    endif (NOT xterm_prog)

    find_program(xmessage_prog NAMES xmessage)
    if (NOT xmessage_prog)
      message("Warning: could not find xmessage")
    endif (NOT xmessage_prog)

    find_program(aplay_prog NAMES aplay)
    if (NOT aplay_prog)
      message("Warning: could not find aplay")
    endif (NOT aplay_prog)

  else()
    message("X11 not found, building tiny version")
    add_definitions(-DTINY_NUBASIC_VER)
  endif()
else()
  add_definitions(-DTINY_NUBASIC_VER)
endif()

add_subdirectory(lib)

if (WITH_IDE)
  include (${CMAKE_ROOT}/Modules/FindGTK2.cmake)
  if(NOT GTK2_FOUND)
    message("Error: GTK2 not found, cannot build gtk2 version")
  else()
    add_subdirectory(ide/gtk)
  endif() 
endif()

include_directories(include ${X11_INCLUDE_DIR})
link_directories(${lib_BINARY_DIR})

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14" )

add_executable(nubasic ${SOURCES})

target_link_libraries(nubasic nubasicinterpreter ${X11_LIBRARIES} -pthread)
