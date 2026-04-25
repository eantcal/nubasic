cmake_minimum_required(VERSION 3.14)  # 3.14+ required for FetchContent_MakeAvailable

project(nuBASIC)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(MAJOR_VERSION 2)
set(MINOR_VERSION 0)
set(PATCH_VERSION 0)

set(NUBASIC_BUILD_NUMBER "$ENV{BUILD_NUMBER}" CACHE STRING
  "Optional build number appended to the nuBASIC version")
if(NUBASIC_BUILD_NUMBER STREQUAL "")
  set(NUBASIC_BUILD_NUMBER 0)
endif()
if(NOT NUBASIC_BUILD_NUMBER MATCHES "^[0-9]+$")
  message(FATAL_ERROR "NUBASIC_BUILD_NUMBER must be numeric, got '${NUBASIC_BUILD_NUMBER}'")
endif()

set(NUBASIC_VERSION_BASE "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
set(NUBASIC_VERSION_DISPLAY "${NUBASIC_VERSION_BASE}")
if(NOT NUBASIC_BUILD_NUMBER STREQUAL "0")
  set(NUBASIC_VERSION_DISPLAY
      "${NUBASIC_VERSION_BASE}.${NUBASIC_BUILD_NUMBER}")
endif()

set(NUBASIC_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/include/util")
file(MAKE_DIRECTORY "${NUBASIC_GENERATED_INCLUDE_DIR}")
configure_file(
  "${CMAKE_SOURCE_DIR}/include/util/config.h.in"
  "${NUBASIC_GENERATED_INCLUDE_DIR}/config.h"
  @ONLY
)

# GCC/Clang-specific global flags (MSVC uses its own conventions)
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20")
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

# Console-only build flag (TINY_NUBASIC_VER) — relevant on Linux/macOS, not Windows.
# A console build strips graphics, sound, mouse and window manager support,
# leaving only the core interpreter. On Windows the GDI console fills the same
# role, so the flag is never set there.
set(_nubasic_console FALSE)  # CMake-level flag tracked alongside TINY_NUBASIC_VER

if (NOT WIN32)
  if (NOT APPLE)
    option(WITH_X11 "X11 graphics/sound" ON)
    option(WITH_IDE "GTK+2 Integrated Development Environment (IDE)" ON)
  else()
    message("On this platform X11 and GTK+2 IDE are not supported — building console-only")
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
      message("X11 not found — building console-only version")
      add_definitions(-DTINY_NUBASIC_VER)
      set(_nubasic_console TRUE)
    endif()
  else()
    add_definitions(-DTINY_NUBASIC_VER)
    set(_nubasic_console TRUE)
  endif()
endif()

# Keep _nubasic_tiny as an alias for backward compatibility with any downstream
# CMake code that tests this variable.
set(_nubasic_tiny ${_nubasic_console})

# Windows: console/win must be added BEFORE add_subdirectory(lib) so the
# nuWinConsole target exists when the interpreter links against it.
if (WIN32)
  option(WITH_WIN_IDE "Windows IDE" ON)

  # Build the GDI console library (always enabled on Windows full builds)
  if (NOT _nubasic_console)
    add_subdirectory(console/win)
  endif()
endif()

add_subdirectory(lib)

# Scintilla editor component (downloaded or local fallback)
# Only needed when building an IDE (Windows or GTK); skip when:
#   - building on macOS (console-only)
#   - building a console-only version (X11/GTK not available on Linux)
if (WIN32 OR (WITH_IDE AND NOT _nubasic_console AND NOT WIN32 AND NOT APPLE))
  include(cmake/ScintillaFetch.cmake)
endif()

# Windows IDE and console executables (depend on lib, so added after it)
if (WIN32)
  if (WITH_WIN_IDE)
    add_subdirectory(ide/win)
  endif()

  if (NOT _nubasic_console)
    add_subdirectory(cli/win)
  endif()
endif()

# GTK IDE for Unix/Linux (skip in console-only mode — GTK/X11 unavailable)
if (WITH_IDE AND NOT _nubasic_console AND NOT WIN32)
  include (${CMAKE_ROOT}/Modules/FindGTK2.cmake)
  if(NOT GTK2_FOUND)
    message("Error: GTK2 not found, cannot build gtk2 version")
  else()
    add_subdirectory(ide/gtk)
  endif()
endif()

include_directories(include
  ${NUBASIC_GENERATED_INCLUDE_DIR}
  include/parser
  include/ast
  include/statements
  include/runtime
  include/types
  include/api
  include/util
  ${X11_INCLUDE_DIR})
link_directories(${lib_BINARY_DIR})

# Build Unix/Linux console executable
if (NOT WIN32)
  set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cli/main.cc")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20" )

  add_executable(nubasic ${SOURCES})
  target_compile_features(nubasic PRIVATE cxx_std_20)

  target_link_libraries(nubasic nuBasicInterpreter ${X11_LIBRARIES} -pthread)

  add_executable(nubasicdebug ${SOURCES})
  target_compile_features(nubasicdebug PRIVATE cxx_std_20)
  target_compile_definitions(nubasicdebug PRIVATE NUBASIC_IDE_DEBUG_CLI=1)
  target_link_libraries(
    nubasicdebug nuBasicInterpreter ${X11_LIBRARIES} -pthread)

  # add the install targets
  install (TARGETS nubasic DESTINATION bin)
  install (TARGETS nubasicdebug DESTINATION bin)
endif()

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
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${NUBASIC_VERSION_DISPLAY}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${NUBASIC_VERSION_DISPLAY}")

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

    # perMachine: installs to Program Files (requires UAC elevation);
    # creates uninstall entry in HKLM so nuBASIC appears in Settings → Apps
    # for all users on the machine.
    set(CPACK_WIX_INSTALL_SCOPE "perMachine")

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

    # WixUI_FeatureTree: shows optional-component selection on every install.
    set(CPACK_WIX_UI_REF "WixUI_FeatureTree")

    # License text shown on the first page of the installer.
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/cmake/License.rtf")

    # WiX v3 Fragment GC silently drops sections that have no symbol
    # reachable from the product root.  Shortcuts in virtual folders
    # (ProgramMenuFolder, DesktopFolder) are injected *after* CPack
    # generates the WXS files by cmake/PatchWixMSI.ps1, which patches
    # directories.wxs and features.wxs and then relinks the MSI.
    # cmake/WixPatch.wxs documents the content for reference only.

    # URLs shown in Add/Remove Programs.
    set(CPACK_WIX_HELP_URL  "https://github.com/eantcal/nubasic")
    set(CPACK_WIX_ABOUT_URL "https://github.com/eantcal/nubasic")

    # 64-bit target: CPack WIX uses CPACK_WIX_SIZEOF_VOID_P (set automatically
    # by CMake) to select ProgramFiles64Folder vs ProgramFilesFolder.

    # ---------------------------------------------------------------------------
    # Optional install components shown in the WixUI_FeatureTree dialog.
    # ---------------------------------------------------------------------------
    set(CPACK_COMPONENTS_ALL "Core;IDE;VSCodeExtension;Examples")

    set(CPACK_COMPONENT_CORE_DISPLAY_NAME   "nuBASIC Runtime")
    set(CPACK_COMPONENT_CORE_DESCRIPTION    "CLI interpreter (nubasic.exe / nubasic_t.exe)")
    set(CPACK_COMPONENT_CORE_REQUIRED       TRUE)

    set(CPACK_COMPONENT_IDE_DISPLAY_NAME    "nuBASIC IDE")
    set(CPACK_COMPONENT_IDE_DESCRIPTION     "Windows Integrated Development Environment")

    set(CPACK_COMPONENT_VSCODEEXTENSION_DISPLAY_NAME  "VS Code Extension")
    set(CPACK_COMPONENT_VSCODEEXTENSION_DESCRIPTION   "Syntax highlighting and debugger for Visual Studio Code")

    set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Example Programs")
    set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION  "Sample nuBASIC source files (.bas)")
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
  foreach(_t nuBasicIDE nuBasicCLI nuBasicGDI SciLexer)
    if(TARGET ${_t})
      list(APPEND _installer_deps ${_t})
    endif()
  endforeach()

  if(TARGET VscodeExtensionBuild)
    list(APPEND _installer_deps VscodeExtensionBuild)
  endif()

  # Locate WiX Toolset v3 candle/light for the post-CPack patch step.
  # Typical install: "C:/Program Files (x86)/WiX Toolset v3.xx/bin/"
  find_program(WIX_CANDLE candle
    HINTS
      "C:/Program Files (x86)/WiX Toolset v3.14/bin"
      "C:/Program Files (x86)/WiX Toolset v3.11/bin"
      "C:/Program Files (x86)/WiX Toolset v3.10/bin"
    DOC "WiX Toolset candle.exe compiler")
  find_program(WIX_LIGHT light
    HINTS
      "C:/Program Files (x86)/WiX Toolset v3.14/bin"
      "C:/Program Files (x86)/WiX Toolset v3.11/bin"
      "C:/Program Files (x86)/WiX Toolset v3.10/bin"
    DOC "WiX Toolset light.exe linker")

  set(_msi_out
      "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}.msi")

  set(_installer_cmds
    COMMAND powershell
            -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/cmake/RunCpackInstaller.ps1"
            -CPackExe   "${CMAKE_CPACK_COMMAND}"
            -Generator  "${NUBASIC_INSTALLER}"
            -Config     "$<CONFIG>"
            -ConfigFile "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
            -BinaryDir  "${CMAKE_BINARY_DIR}"
            -SourceDir  "${CMAKE_SOURCE_DIR}"
            -MsiOutput  "${_msi_out}"
            -Candle     "${WIX_CANDLE}"
            -Light      "${WIX_LIGHT}"
  )

  add_custom_target(CreateInstaller
    ${_installer_cmds}
    DEPENDS ${_installer_deps}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Building ${NUBASIC_INSTALLER} installer for $<CONFIG>..."
    VERBATIM
  )
  set_property(TARGET CreateInstaller PROPERTY FOLDER "Installer")

  # Move the standard CPack PACKAGE target into the same solution folder.
  if(TARGET PACKAGE)
    set_property(TARGET PACKAGE PROPERTY FOLDER "Installer")
  endif()
endif()

ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

# ---------------------------------------------------------------------------
# RunTests target — runs all tests/test_*.bas through the interpreter in
# text mode (-t) and prints a pass/fail report to stdout.
#
# From Visual Studio: right-click RunTests → Build
# From command line:  cmake --build <builddir> --target RunTests --config Release
#
# The target depends on nuBasicCLI so the interpreter is always up to date
# before the suite runs.
# ---------------------------------------------------------------------------
if(WIN32 AND MSVC)
  add_custom_target(RunTests
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tests/run_tests.ps1"
            -Interpreter "$<TARGET_FILE:nuBasicCLI>"
            -TestDir     "${CMAKE_SOURCE_DIR}/tests"
    DEPENDS nuBasicCLI
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Running nuBASIC regression tests..."
    VERBATIM
  )
  set_property(TARGET RunTests PROPERTY FOLDER "Tests")

  add_custom_target(RunRosettaBenchmark
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/benchmarks/run_rosetta.ps1"
            -Interpreter "$<TARGET_FILE:nuBasicCLI>"
            -Script "${CMAKE_SOURCE_DIR}/examples/rosetta.bas"
            -Iterations 1
            -Warmup 1
    DEPENDS nuBasicCLI
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Running nuBASIC Rosetta benchmark..."
    VERBATIM
  )
  set_property(TARGET RunRosettaBenchmark PROPERTY FOLDER "Tests")
elseif(NOT WIN32)
  add_custom_target(RunTests
    COMMAND bash "${CMAKE_SOURCE_DIR}/tests/run_tests.sh"
            --interpreter "$<TARGET_FILE:nubasic>"
            --testdir     "${CMAKE_SOURCE_DIR}/tests"
    DEPENDS nubasic
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Running nuBASIC regression tests..."
    VERBATIM
  )
endif()

# ---------------------------------------------------------------------------
# VS Code extension (vscode-nubasic) — optional
# Requires Node.js + npm. Emits a warning and skips if not found.
#
# Targets (grouped under "VSCode" in Solution Explorer):
#   VscodeExtensionBuild   — npm install + compile TypeScript + package .vsix
#   VscodeExtensionInstall — install the .vsix into the local VS Code instance
# ---------------------------------------------------------------------------
find_program(NODE_EXE node DOC "Node.js runtime (required for VS Code extension)")
find_program(NPM_EXE NAMES npm.cmd npm DOC "npm (required for VS Code extension)")

if(NOT NODE_EXE OR NOT NPM_EXE)
  message(WARNING
    "Node.js or npm not found — VS Code extension targets are disabled. "
    "Install Node.js from https://nodejs.org to enable them.")
else()
  set(_ext_dir "${CMAKE_SOURCE_DIR}/vscode-nubasic")
  set(_vsix "${_ext_dir}/nubasic-0.1.4.vsix")

  # Bundle the pre-built VSIX and its platform-specific install helper.
  install(FILES "${_vsix}"
          DESTINATION bin OPTIONAL COMPONENT VSCodeExtension)
  if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/cmake/install-vscode-ext.ps1"
            DESTINATION bin COMPONENT VSCodeExtension)
  else()
    install(FILES "${CMAKE_SOURCE_DIR}/cmake/install-vscode-ext.sh"
            DESTINATION bin
            PERMISSIONS
              OWNER_READ OWNER_WRITE OWNER_EXECUTE
              GROUP_READ GROUP_EXECUTE
              WORLD_READ WORLD_EXECUTE
            COMPONENT VSCodeExtension)
  endif()

  add_custom_target(VscodeExtensionBuild
    BYPRODUCTS "${_vsix}"
    COMMAND "${NPM_EXE}" install
    COMMAND "${NPM_EXE}" run package
    WORKING_DIRECTORY "${_ext_dir}"
    COMMENT "Building VS Code extension (vscode-nubasic)..."
    VERBATIM
  )
  set_property(TARGET VscodeExtensionBuild PROPERTY FOLDER "VSCode")

  # Locate the VS Code CLI to enable the install target
  if(WIN32)
    find_program(VSCODE_CLI
      NAMES code.cmd
      HINTS
        "$ENV{LOCALAPPDATA}/Programs/Microsoft VS Code/bin"
        "$ENV{ProgramFiles}/Microsoft VS Code/bin"
      DOC "VS Code CLI (code.cmd)")
  else()
    find_program(VSCODE_CLI NAMES code code-insiders DOC "VS Code CLI")
  endif()

  if(NOT VSCODE_CLI)
    message(WARNING
      "VS Code CLI not found — VscodeExtensionInstall target is disabled. "
      "Ensure 'code' (or code.cmd) is on PATH to enable it.")
  else()
    add_custom_target(VscodeExtensionInstall
      COMMAND "${VSCODE_CLI}" --install-extension "${_vsix}" --force
      WORKING_DIRECTORY "${_ext_dir}"
      DEPENDS VscodeExtensionBuild
      COMMENT "Installing VS Code extension into local VS Code..."
      VERBATIM
    )
    set_property(TARGET VscodeExtensionInstall PROPERTY FOLDER "VSCode")
  endif()
endif()
