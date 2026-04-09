# ScintillaFetch.cmake
# Downloads Scintilla and Lexilla via CMake FetchContent and creates build targets.
#
# Releases: https://www.scintilla.org/
#
# Cache variables (override on cmake command line):
#   SCINTILLA_VERSION  - Scintilla version string (default "5.5.3")
#   LEXILLA_VERSION    - Lexilla version string   (default "5.4.3")
#   SCINTILLA_LOCAL    - Set to ON to use the bundled 4.x copy instead of downloading
#
# Exported variables (available to parent scope after include):
#   SCINTILLA_INCLUDE_DIRS  - Scintilla public headers
#   LEXILLA_INCLUDE_DIRS    - Lexilla/SciLexer public headers

include(FetchContent)

option(SCINTILLA_LOCAL "Use bundled Scintilla source instead of downloading" OFF)

# ---------------------------------------------------------------------------
# Source resolution
# ---------------------------------------------------------------------------
if(NOT SCINTILLA_LOCAL)
    set(SCINTILLA_VERSION "5.5.3" CACHE STRING "Scintilla version to fetch (e.g. 5.5.3)")
    set(LEXILLA_VERSION   "5.4.3" CACHE STRING "Lexilla version to fetch  (e.g. 5.4.3)")

    # Convert "X.Y.Z" -> "XYZ" for the scintilla.org filename convention
    string(REPLACE "." "" _sci_ver "${SCINTILLA_VERSION}")
    string(REPLACE "." "" _lex_ver "${LEXILLA_VERSION}")

    message(STATUS "Scintilla: fetching version ${SCINTILLA_VERSION} (scintilla${_sci_ver}.zip)")
    message(STATUS "Lexilla:   fetching version ${LEXILLA_VERSION}   (lexilla${_lex_ver}.zip)")

    # DOWNLOAD_EXTRACT_TIMESTAMP requires CMake >= 3.24; guard it to stay
    # compatible with the CMake 3.22 shipped by Ubuntu 22.04.
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        set(_fetch_extra DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    else()
        set(_fetch_extra "")
    endif()

    FetchContent_Declare(scintilla_dl
        URL "https://www.scintilla.org/scintilla${_sci_ver}.zip"
        ${_fetch_extra}
    )
    FetchContent_Declare(lexilla_dl
        URL "https://www.scintilla.org/lexilla${_lex_ver}.zip"
        ${_fetch_extra}
    )
    FetchContent_MakeAvailable(scintilla_dl lexilla_dl)

    # Some archives extract into a subdirectory; handle both layouts.
    set(_sci_root "${scintilla_dl_SOURCE_DIR}")
    if(EXISTS "${_sci_root}/scintilla/include/Scintilla.h")
        set(_sci_root "${_sci_root}/scintilla")
    endif()

    set(_lex_root "${lexilla_dl_SOURCE_DIR}")
    if(EXISTS "${_lex_root}/lexilla/include/SciLexer.h")
        set(_lex_root "${_lex_root}/lexilla")
    endif()

    set(_build_scintilla TRUE)
else()
    # Fallback: bundled Scintilla 4.x (headers + prebuilt DLL on Windows)
    set(_sci_root "${CMAKE_SOURCE_DIR}/ide/scintilla")
    set(_lex_root "${CMAKE_SOURCE_DIR}/ide/scintilla")
    set(_build_scintilla FALSE)
    message(STATUS "Scintilla: using bundled copy at ${_sci_root}")
endif()

# Publish include directories to callers
set(SCINTILLA_INCLUDE_DIRS "${_sci_root}/include" CACHE PATH "Scintilla include directory" FORCE)
set(LEXILLA_INCLUDE_DIRS   "${_lex_root}/include" CACHE PATH "Lexilla include directory"   FORCE)

# ---------------------------------------------------------------------------
# Build targets (only when using the downloaded version)
# ---------------------------------------------------------------------------
if(_build_scintilla)

    if(WIN32)
        # -----------------------------------------------------------------------
        # Windows: build SciLexer.dll (combined Scintilla + Lexilla)
        # -----------------------------------------------------------------------
        file(GLOB _sci_core_src   "${_sci_root}/src/*.cxx")
        file(GLOB _lex_core_src   "${_lex_root}/src/Lexilla.cxx")
        file(GLOB _lex_lib_src    "${_lex_root}/lexlib/*.cxx")
        file(GLOB _lex_lexers_src "${_lex_root}/lexers/*.cxx")

        # Explicit win32 files to avoid pulling in test/unused sources
        set(_sci_win_src
            "${_sci_root}/win32/PlatWin.cxx"
            "${_sci_root}/win32/ScintillaWin.cxx"
            "${_sci_root}/win32/ScintillaDLL.cxx"
        )
        if(EXISTS "${_sci_root}/win32/HanjaDic.cxx")
            list(APPEND _sci_win_src "${_sci_root}/win32/HanjaDic.cxx")
        endif()

        add_library(SciLexer SHARED
            ${_sci_core_src}
            ${_sci_win_src}
            ${_lex_core_src}
            ${_lex_lib_src}
            ${_lex_lexers_src}
        )

        target_include_directories(SciLexer PRIVATE
            "${_sci_root}/include"
            "${_sci_root}/src"
            "${_lex_root}/include"
            "${_lex_root}/lexlib"
        )

        target_compile_definitions(SciLexer PRIVATE
            WIN32
            SCI_LEXER
            _CRT_SECURE_NO_DEPRECATE
            _SCL_SECURE_NO_WARNINGS
            _WINDOWS
            _USRDLL
        )

        target_compile_features(SciLexer PRIVATE cxx_std_17)

        if(MSVC)
            # Suppress warnings from third-party code; ensure UTF-8 source encoding
            target_compile_options(SciLexer PRIVATE /W0 /utf-8)
            set_property(TARGET SciLexer PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()

        target_link_libraries(SciLexer PRIVATE
            gdi32 imm32 ole32 oleaut32 msimg32
        )

        # Output name must match the LoadLibrary("SciLexer.DLL") call in the IDE
        set_target_properties(SciLexer PROPERTIES
            OUTPUT_NAME "SciLexer"
            PREFIX      ""
        )

    else()
        # -----------------------------------------------------------------------
        # Linux/GTK: build as a static library (mirrors the old `make` approach)
        # -----------------------------------------------------------------------
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

        file(GLOB _sci_core_src   "${_sci_root}/src/*.cxx")
        file(GLOB _sci_gtk_src    "${_sci_root}/gtk/*.cxx" "${_sci_root}/gtk/*.c")
        file(GLOB _lex_core_src   "${_lex_root}/src/Lexilla.cxx")
        file(GLOB _lex_lib_src    "${_lex_root}/lexlib/*.cxx")
        file(GLOB _lex_lexers_src "${_lex_root}/lexers/*.cxx")

        add_library(scintilla STATIC
            ${_sci_core_src}
            ${_sci_gtk_src}
            ${_lex_core_src}
            ${_lex_lib_src}
            ${_lex_lexers_src}
        )

        target_include_directories(scintilla PRIVATE
            "${_sci_root}/include"
            "${_sci_root}/src"
            "${_lex_root}/include"
            "${_lex_root}/lexlib"
            ${GTK2_INCLUDE_DIRS}
        )

        target_compile_definitions(scintilla PRIVATE
            GTK
            SCI_LEXER
        )

        target_compile_features(scintilla PRIVATE cxx_std_17)

        # Suppress warnings from third-party Scintilla/GTK2 code
        target_compile_options(scintilla PRIVATE -w)
    endif()

endif() # _build_scintilla

