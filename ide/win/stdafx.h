//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#pragma once

#include "targetver.h"

#include <windows.h>
#include <windowsx.h>
#include <WinUser.h>

#include <Commctrl.h>
#include <WinUser.h>
#include <malloc.h>
#include <memory.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <errno.h>
#include <fcntl.h>
#include <io.h>

#include "SciLexer.h"
#include "Scintilla.h"
#include "resource.h"

#include "config.h"

#include <algorithm>
#include <string>
#include <vector>

#define EDITOR_DLL_NAME "SciLexer.DLL"
#define EDITOR_RESOURCE_NAME "nuBasicEditor"
#define EDITOR_FILE_FILTER "nuBASIC (.bas)\0*.bas;\0All Files (*.*)\0*.*\0\0"
#define EDITOR_NONAME_FILE "noname.bas"
#define EDITOR_VERSION VERSION
#define EDITOR_AUTHOR PACKAGE_BUGREPORT
#define EDITOR_INFO                                                            \
    EDITOR_RESOURCE_NAME " (ver. " EDITOR_VERSION ") by " EDITOR_AUTHOR

#ifdef _M_AMD64
#define EDITOR_ABOUT_TEXT                                                      \
    "nuBASIC IDE for x64 platform\n"                                           \
    "See README for more information."
#else
#define EDITOR_ABOUT_TEXT                                                      \
    "nuBASIC IDE for 32 bit platform\n"                                        \
    "See README for more information."
#endif
#define EDITOR_DEF_FONT "Lucida Console"
#define EDITOR_DEF_SIZE 10

#define TOOLBAR_HEIGHT 32
