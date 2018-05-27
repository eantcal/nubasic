cmake_minimum_required(VERSION 2.8.12)

include (${CMAKE_ROOT}/Modules/FindX11.cmake)
include (${CMAKE_ROOT}/Modules/FindGTK2.cmake)

if(NOT GTK2_FOUND)
  message(FATAL_ERROR "Error: GTK2 not found")
else()
  add_definitions(-DGTK)
endif() 

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
  message(FATAL_ERROR "Error: X11 not found")
endif()


add_subdirectory(mipjson)

add_custom_target(
   scintilla
   COMMAND make
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/gtk
)


set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GTK2_DEFINITIONS} -std=c++14" )

include_directories(
    ../../ 
    ../../include 
    mipjson/include 
    mipjson/miptknzr/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/include 
    ${X11_INCLUDE_DIR} 
    ${GTK2_INCLUDE_DIRS}
)

link_directories(${lib_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/bin" )

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_executable(nubasicide ${SOURCES})
add_dependencies(nubasicide scintilla)

target_link_libraries(nubasicide nubasicinterpreter "${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/bin/scintilla.a" mipjson ${X11_LIBRARIES} ${GTK2_LIBRARIES} -pthread)
