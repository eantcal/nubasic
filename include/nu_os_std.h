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
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


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

}


/* -------------------------------------------------------------------------- */

#endif //__NU_OS_STD_H__
