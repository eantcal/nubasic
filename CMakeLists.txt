cmake_minimum_required(VERSION 2.8.12)

project(nuBASIC)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )

add_executable(nubasic ${SOURCES})
target_compile_features(nubasic PRIVATE cxx_std_17)

target_link_libraries(nubasic nubasicinterpreter ${X11_LIBRARIES} -pthread)

# add the install targets
install (TARGETS nubasic DESTINATION bin)

SET(MAJOR_VERSION 1)
SET(MINOR_VERSION 50)
SET(PATCH_VERSION 0)
 
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
INCLUDE(InstallRequiredSystemLibraries)
 
SET(CPACK_SET_DESTDIR "on")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
SET(CPACK_GENERATOR "DEB")
 
SET(CPACK_PACKAGE_DESCRIPTION "nuBASIC")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "nuBASIC Interpreter")
SET(CPACK_PACKAGE_VENDOR "Antonino Calderone")
SET(CPACK_PACKAGE_CONTACT "antonino.calderone@gmail.com")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.nubasic.eu")

SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
 
IF(USE_X11)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libx11-dev (>=2:1.3.2)")
ENDIF()

IF(USE_GTK2)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libgtk2.0-dev (>=2.20.1), libglib2.0-dev (>=2.24.1), libpango1.0-dev (>=1.28.0), libatk1.0-dev (>=1.30.0), libcairo2-dev (>=1.8.10)")
ENDIF()

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1.3), libgcc1 (>= 1:4.1.1)")
 
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
 
SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)
INCLUDE(CPack)
 
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
