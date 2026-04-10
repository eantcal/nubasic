//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STRING_TOOL_H__
#define __NU_STRING_TOOL_H__


/* -------------------------------------------------------------------------- */

#include <iomanip>
#include <sstream>
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

inline std::string to_string(double item)
{
    std::ostringstream oss;
    oss << std::setprecision(15) << item;

    auto out = oss.str();
    const auto dot = out.find('.');

    if (dot != std::string::npos) {
        while (!out.empty() && out.back() == '0') {
            out.pop_back();
        }

        if (!out.empty() && out.back() == '.') {
            out.pop_back();
        }
    }

    if (out == "-0") {
        out = "0";
    }

    return out;
}

inline std::string to_string(float item)
{
    return to_string(static_cast<double>(item));
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
