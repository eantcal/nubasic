/*
 *  This file is part of nuBASIC
 *
 *  nuBASIC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  nuBASIC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with nuBASIC; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
 *
 *  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
 *
 */

#pragma once

#include "targetver.h"

#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <WinUser.h>
#include <stdio.h>
#include <richedit.h>
#include <Commctrl.h>

#include <io.h>
#include <fcntl.h>
#include <errno.h>

#include "resource.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "config.h"

#include <string>
#include <vector>
#include <algorithm>

#define EDITOR_DLL_NAME "SciLexer.DLL"
#define EDITOR_RESOURCE_NAME "nuBasicEditor"
#define EDITOR_FILE_FILTER "nuBASIC (.bas)\0*.bas;\0All Files (*.*)\0*.*\0\0"
#define EDITOR_NONAME_FILE "noname.bas"
#define EDITOR_VERSION VERSION
#define EDITOR_AUTHOR PACKAGE_BUGREPORT
#define EDITOR_INFO EDITOR_RESOURCE_NAME " (ver. " EDITOR_VERSION ") by " EDITOR_AUTHOR

#ifdef _M_AMD64
#define EDITOR_ABOUT_TEXT "nuBASIC IDE for x64 platform\n" \
                          "See README for more information."
#else
#define EDITOR_ABOUT_TEXT "nuBASIC IDE for 32 bit platform\n" \
                          "See README for more information."
#endif
#define EDITOR_DEF_FONT "Lucida Console"
#define EDITOR_DEF_SIZE 10

#define TOOLBAR_HEIGHT 32
