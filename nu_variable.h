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

#ifndef __NU_VARIABLE_H__
#define __NU_VARIABLE_H__

#include "nu_cpp_lang.h"
#include "nu_reserved_keywords.h"

#include <set>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct variable_t {
    enum class type_t {
        UNDEFINED,
        INTEGER,
        FLOAT,
        DOUBLE,
        STRING,
        BYTEVECTOR,
        BOOLEAN,
        LONG64,
        STRUCT
    };


    static type_t type_by_name(const std::string& name);
    static bool is_valid_name(std::string name, bool ignore_builtin);
    static type_t type_by_typename(std::string name);
    static std::string typename_by_type(type_t type);


    static inline bool is_number(type_t t) noexcept
    {
        return t == type_t::LONG64 || t == type_t::INTEGER || t == type_t::FLOAT
            || t == type_t::DOUBLE || t == type_t::BOOLEAN;
    }


    static inline bool is_float(type_t t) noexcept
    {
        return t == type_t::FLOAT || t == type_t::DOUBLE;
    }


    static inline bool is_integral(type_t t) noexcept
    {
        return t == type_t::LONG64 || t == type_t::INTEGER
            || t == type_t::BOOLEAN;
    }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_VARIABLE_H__
