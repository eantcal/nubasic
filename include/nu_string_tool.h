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

#ifndef __NU_STRING_TOOL_H__
#define __NU_STRING_TOOL_H__


/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "nu_cpp_lang.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

//! Parses s interpreting its content as a floating-point number
//! which is returned as a value of type float

inline float stof(const std::string& s) { return (float)atof(s.c_str()); }


/* -------------------------------------------------------------------------- */

//! Parses s interpreting its content as a floating-point number
//! which is returned as a value of type float
inline double stod(const std::string& s) { return std::stod(s); }


/* -------------------------------------------------------------------------- */

//! Parses s interpreting its content as an integral number
//! which is returned as an int value
inline int stoi(const std::string& s) { return std::stoi(s); }


/* -------------------------------------------------------------------------- */

//! Parses s interpreting its content as an long integral number
//! which is returned as an int value
inline long long stoll(const std::string& s) { return std::stoll(s); }


/* -------------------------------------------------------------------------- */

//! Returns a string with the representation of item
template <class T> inline std::string to_string(const T& item)
{
    return std::to_string(item);
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#if _MSC_VER

#define strcasecmp _stricmp

#elif __MINGW__

#include <algorithm>
#include <cctype>
#include <strings.h>

//! Compares s1 and s2 without sensitivity to case

int strcasecmp(const char* s1, const char* s2);

/* -------------------------------------------------------------------------- */


#endif // _MSC_VER


/* -------------------------------------------------------------------------- */

#endif //__NU_STRING_TOOL_H__
