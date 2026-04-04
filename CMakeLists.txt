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

# Windows: console/win must be added BEFORE add_subdirectory(lib) so the
# nuWinConsole target exists when the interpreter links against it.
if (WIN32)
  option(WITH_WIN_IDE "Windows IDE" ON)

  # Build the GDI console library (always enabled on Windows non-TINY builds)
  if (NOT _nubasic_tiny)
    add_subdirectory(console/win)
  endif()
endif()

add_subdirectory(lib)

# Scintilla editor component (downloaded or local fallback)
# Only needed when building an IDE (Windows or GTK); skip on macOS CLI-only builds.
if (WIN32 OR (WITH_IDE AND NOT WIN32 AND NOT APPLE))
  include(cmake/ScintillaFetch.cmake)
endif()

# Windows IDE and console executables (depend on lib, so added after it)
if (WIN32)
  if (WITH_WIN_IDE)
    add_subdirectory(ide/win)
  endif()

  if (NOT _nubasic_tiny)
    add_subdirectory(cli/win)
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
  file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cli/unix/*.cc")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )

  add_executable(nubasic ${SOURCES})
  target_compile_features(nubasic PRIVATE cxx_std_17)

  target_link_libraries(nubasic nuBasicInterpreter ${X11_LIBRARIES} -pthread)

  # add the install targets
  install (TARGETS nubasic DESTINATION bin)
endif()

SET(MAJOR_VERSION 1)
SET(MINOR_VERSION 61)
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
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/eantcal/nubasic")

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

if(WIN32)
  # ---------------------------------------------------------------------------
  # Windows MSI installer via CPack WIX generator.
  #
  # Prerequisites:
  #   WiX Toolset v3.x  –  https://wixtoolset.org/releases/
  #   candle.exe and light.exe must be on PATH.
  #
  # Fallback: set NUBASIC_INSTALLER=NSIS to use the old NSIS-based setup.exe
  #   (requires makensis.exe on PATH instead).
  #
  # Build the installer:
  #   cmake --build <builddir> --config Release
  #   cd <builddir>
  #   cpack -G WIX -C Release          # or -G NSIS with the fallback
  # ---------------------------------------------------------------------------

  set(NUBASIC_INSTALLER "WIX" CACHE STRING
      "Installer generator for Windows: WIX (default, MSI) or NSIS (legacy setup.exe)")
  set_property(CACHE NUBASIC_INSTALLER PROPERTY STRINGS WIX NSIS)

  set(CPACK_SET_DESTDIR OFF)
  set(CPACK_PACKAGING_INSTALL_PREFIX "")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "nuBASIC")

  if(NUBASIC_INSTALLER STREQUAL "NSIS")
    # ---- NSIS fallback (legacy) ----
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "nuBASIC")
    set(CPACK_NSIS_PACKAGE_NAME "nuBASIC")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
      WriteRegStr HKCU 'Software\\\\nuBASIC' 'InstallDir' \\\"$INSTDIR\\\"
      WriteRegStr HKCU 'Software\\\\nuBASIC' 'ExamplesDir' \\\"$INSTDIR\\\\examples\\\"
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
      DeleteRegKey HKCU 'Software\\\\nuBASIC'
    ")
  else()
    # ---- WIX MSI (default) ----
    set(CPACK_GENERATOR "WIX")

    # UPGRADE_GUID must never change across versions of nuBASIC — it is the
    # stable product identity that Windows Installer uses to detect upgrades
    # and prevent duplicate installations.
    set(CPACK_WIX_UPGRADE_GUID "A1B2C3D4-E5F6-4789-8ABC-DEF012345678")

    # Application icon shown in the installer and in Add/Remove Programs.
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/ide/win/nuBasic.ico")

    # Banner (493x58) and dialog (493x312) bitmaps for the WiX installer UI.
    # These are pre-scaled versions of ide/win/nubasic.bmp: the original image
    # is centered on a white background at the correct size so WiX does not
    # stretch or deform it.
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/cmake/nubasic_banner.bmp")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/cmake/nubasic_dialog.bmp")

    # WixUI_InstallDir: lets the user choose the installation directory.
    set(CPACK_WIX_UI_REF "WixUI_InstallDir")

    # License text shown on the first page of the installer.
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/cmake/License.rtf")

    # Extra WiX source: adds Start Menu shortcuts, runtime registry keys and the
    # .bas file-type association.  See cmake/WixPatch.wxs for details.
    set(CPACK_WIX_EXTRA_SOURCES "${CMAKE_SOURCE_DIR}/cmake/WixPatch.wxs")

    # URLs shown in Add/Remove Programs.
    set(CPACK_WIX_HELP_URL  "https://github.com/eantcal/nubasic")
    set(CPACK_WIX_ABOUT_URL "https://github.com/eantcal/nubasic")

    # 64-bit target: CPack WIX uses CPACK_WIX_SIZEOF_VOID_P (set automatically
    # by CMake) to select ProgramFiles64Folder vs ProgramFilesFolder.
  endif()

else()
  SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)
endif()
 
INCLUDE(CPack)

# ---------------------------------------------------------------------------
# Visual Studio solution target: "CreateInstaller"
#
# Lets the developer build the MSI (or NSIS setup) directly from Visual
# Studio without leaving the IDE:
#   • Right-click CreateInstaller → Build
#   • Or: cmake --build <builddir> --target CreateInstaller --config Release
#
# The target depends on all installed executables, so the binaries are
# guaranteed to be up to date before cpack runs.  $<CONFIG> passes the
# active VS configuration (Release / Debug) to cpack so the correct
# binaries are collected.
#
# Both CreateInstaller and the CPack-generated PACKAGE target are grouped
# under the "Installer" folder in Solution Explorer.
# ---------------------------------------------------------------------------
if(WIN32 AND MSVC)
  # Collect the targets that must be built before packaging.
  set(_installer_deps "")
  foreach(_t nuBasicIDE nuBasicCLI nuWinConsole_test SciLexer)
    if(TARGET ${_t})
      list(APPEND _installer_deps ${_t})
    endif()
  endforeach()

  add_custom_target(CreateInstaller
    COMMAND "${CMAKE_CPACK_COMMAND}"
            -G "${NUBASIC_INSTALLER}"
            -C "$<CONFIG>"
            --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
    DEPENDS ${_installer_deps}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Building ${NUBASIC_INSTALLER} installer (${NUBASIC_INSTALLER}) for $<CONFIG>..."
    VERBATIM
  )
  set_property(TARGET CreateInstaller PROPERTY FOLDER "Installer")

  # Move the standard CPack PACKAGE target into the same solution folder.
  if(TARGET PACKAGE)
    set_property(TARGET PACKAGE PROPERTY FOLDER "Installer")
  endif()
endif()

ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
