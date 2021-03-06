//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_OS_STD_H__
#define __NU_OS_STD_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <string>
#include <type_traits>
#include <vector>

#include "nu_string_tool.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

//! Sleeps for the specified number of seconds
void _os_delay(int s);

//! Sleeps for the specified number of milliseconds
void _os_mdelay(int ms);

//! Beeps the pc speaker
void _os_beep();

//! Changes working directory
bool _os_change_dir(const std::string& dir);

//! Retrieves the current working directory
std::string _os_get_working_dir();


/* -------------------------------------------------------------------------- */

// Random generator

void _os_randomize();


// Date and time

//! Returns local-time day in the range 1-31
int _os_get_day();

//! Returns local-time month in the range 1-12
int _os_get_month();

//! Returns local-time year since 1900
int _os_get_year();

//! Returns local-time week day in the range 0-6 (0 - Sunday)
int _os_get_wday();

//! Returns local-time year day in the range 0-365
int _os_get_yday();

//! Returns local-time hour in the range 0-23
int _os_get_hour();

//! Returns local-time minute of hour in the range 0-59
int _os_get_min();

//! Returns local-time second of hour in the range 0-59
int _os_get_sec();

//! Returns local-time in seconds since 1st of Gen-1900
int _os_get_time();

//! Returns local-time in the format "DDD MMM DN HH:MM:SS YYYY"
std::string _os_get_systime();

//! Deletes a file
int _os_erase_file(const std::string& filepath);

//! Deletes a directory
int _os_erase_dir(const std::string& filepath);

//! Makes a directory
int _os_make_dir(const std::string& filepath);

#if defined(__MINGW32__)
int _os_setenv(const char* var, const char* val);
#elif defined(_MSC_VER)
#define _os_setenv(_VAR, _VAL) _putenv_s(_VAR, _VAL)
#else
#define _os_setenv(_VAR, _VAL) setenv(_VAR, _VAL, 1)
#endif

#if defined(__MINGW32__)
int _os_unsetenv(const char* var);
#elif defined(_MSC_VER)
#define _os_unsetenv(_VAR) _putenv_s(_VAR, "")
#else
#define _os_unsetenv(_VAR) unsetenv(_VAR)
#endif

/* -------------------------------------------------------------------------- */

// Returns application path
std::string _os_get_app_path();


/* -------------------------------------------------------------------------- */

enum {
    vk_BackSpace = 1,
    vk_Tab,
    vk_Return,
    vk_Escape,
    vk_Space,
    vk_PageUp,
    vk_PageDown,
    vk_End,
    vk_Home,
    vk_Left,
    vk_Up,
    vk_Right,
    vk_Down,
    vk_Insert,
    vk_Delete
};

//! Reads the key pressed from input and returns 
//! it as an int (special keys are mapped as vk_xxxx), or EOF in case of error.
int _os_get_vkey();


/* -------------------------------------------------------------------------- */

} // nu


/* -------------------------------------------------------------------------- */

#endif //__NU_OS_STD_H__
