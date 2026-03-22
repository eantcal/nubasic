cmake_minimum_required(VERSION 3.14)  # 3.14+ required for FetchContent_MakeAvailable

project(nuBASIC)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# GCC/Clang-specific global flags (MSVC uses its own conventions)
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# When using Ninja+MSVC from a shell without vcvarsall.bat (e.g. plain Git Bash),
# the INCLUDE env var is empty and cl.exe cannot find standard headers.
# Detect and inject the paths explicitly for all targets.
if(MSVC AND "$ENV{INCLUDE}" STREQUAL "")
  # When using Ninja+MSVC without vcvarsall.bat, both INCLUDE and LIB are empty.
  # Detect the compiler architecture (last component of the binary dir).
  get_filename_component(_cl_arch   "${CMAKE_CXX_COMPILER}" DIRECTORY) # .../x86 or .../x64
  get_filename_component(_arch_name "${_cl_arch}"           NAME)      # "x86" or "x64"
  get_filename_component(_cl_host   "${_cl_arch}"           DIRECTORY)
  get_filename_component(_cl_bin    "${_cl_host}"           DIRECTORY)
  get_filename_component(_msvc_root "${_cl_bin}"            DIRECTORY) # MSVC <ver> dir

  # --- Include paths ---
  if(EXISTS "${_msvc_root}/include")
    include_directories(SYSTEM "${_msvc_root}/include")
  endif()

  # --- Link paths ---
  if(EXISTS "${_msvc_root}/lib/${_arch_name}")
    link_directories("${_msvc_root}/lib/${_arch_name}")
  endif()

  # Windows SDK include + lib paths
  foreach(_wk IN ITEMS
      "C:/Program Files (x86)/Windows Kits/10"
      "C:/Program Files/Windows Kits/10")
    if(EXISTS "${_wk}/Include")
      file(GLOB _sdk_vers LIST_DIRECTORIES true "${_wk}/Include/10.*")
      list(SORT _sdk_vers ORDER DESCENDING)
      if(_sdk_vers)
        list(GET _sdk_vers 0 _sdk_inc)
        string(REGEX REPLACE ".*/Include/" "" _sdk_ver "${_sdk_inc}")
        set(_sdk_lib "${_wk}/Lib/${_sdk_ver}")

        foreach(_sub IN ITEMS ucrt um shared)
          if(EXISTS "${_sdk_inc}/${_sub}")
            include_directories(SYSTEM "${_sdk_inc}/${_sub}")
          endif()
        endforeach()
        foreach(_sub IN ITEMS ucrt um)
          if(EXISTS "${_sdk_lib}/${_sub}/${_arch_name}")
            link_directories("${_sdk_lib}/${_sub}/${_arch_name}")
          endif()
        endforeach()
        break()
      endif()
    endif()
  endforeach()
endif()

# X11 / TINY mode — only relevant on Linux/macOS, not Windows.
# On Windows the GDI console fills the same role; forcing TINY there because
# X11 is absent would be wrong.
set(_nubasic_tiny FALSE)  # CMake-level flag tracked alongside the preprocessor define

if (NOT WIN32)
  if (NOT APPLE)
    option(WITH_X11 "X11 graphics/sound" ON)
    option(WITH_IDE "GTK+2 Integrated Development Environment (IDE)" ON)
  else()
    message("On this platform we don't support X11 and GTK+2 IDE")
  endif()

  if (WITH_X11)
    include (${CMAKE_ROOT}/Modules/FindX11.cmake)
    if(X11_FOUND)
      add_definitions(-DCMAKE_HAS_X)

      find_program(xterm_prog NAMES xterm)
      if (NOT xterm_prog)
        message("Warning: could not find xterm")
      endif()

      find_program(xmessage_prog NAMES xmessage)
      if (NOT xmessage_prog)
        message("Warning: could not find xmessage")
      endif()

      find_program(aplay_prog NAMES aplay)
      if (NOT aplay_prog)
        message("Warning: could not find aplay")
      endif()

    else()
      message("X11 not found, building tiny version")
      add_definitions(-DTINY_NUBASIC_VER)
      set(_nubasic_tiny TRUE)
    endif()
  else()
    add_definitions(-DTINY_NUBASIC_VER)
    set(_nubasic_tiny TRUE)
  endif()
endif()

# Windows: nuwinconsole must be added BEFORE add_subdirectory(lib) so the
# nuwinconsole target exists when the interpreter links against it.
if (WIN32)
  option(WITH_WIN_IDE "Windows IDE" ON)

  # Build the GDI console library (always enabled on Windows non-TINY builds)
  if (NOT _nubasic_tiny)
    add_subdirectory(nuwinconsole)
  endif()
endif()

add_subdirectory(lib)

# Scintilla editor component (downloaded or local fallback)
include(cmake/ScintillaFetch.cmake)

# Windows IDE and console executables (depend on lib, so added after it)
if (WIN32)
  if (WITH_WIN_IDE)
    add_subdirectory(ide/win)
  endif()

  if (NOT _nubasic_tiny)
    add_subdirectory(nuBasicCLI)
  endif()
endif()

# GTK IDE for Unix/Linux
if (WITH_IDE AND NOT WIN32)
  include (${CMAKE_ROOT}/Modules/FindGTK2.cmake)
  if(NOT GTK2_FOUND)
    message("Error: GTK2 not found, cannot build gtk2 version")
  else()
    add_subdirectory(ide/gtk)
  endif() 
endif()

include_directories(include ${X11_INCLUDE_DIR})
link_directories(${lib_BINARY_DIR})

# Build Unix/Linux console executable
if (NOT WIN32)
  file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )

  add_executable(nubasic ${SOURCES})
  target_compile_features(nubasic PRIVATE cxx_std_17)

  target_link_libraries(nubasic nubasicinterpreter ${X11_LIBRARIES} -pthread)

  # add the install targets
  install (TARGETS nubasic DESTINATION bin)
endif()

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
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.eantcal.eu/nubasic")

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
