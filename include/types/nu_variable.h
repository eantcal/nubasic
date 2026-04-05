//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

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
        DOUBLE,
        STRING,
        BYTEVECTOR,
        BOOLEAN,
        STRUCT,
        ANY,
        OBJECT
    };


    static type_t type_by_name(const std::string& name);
    static bool is_valid_name(std::string name, bool ignore_builtin);
    static type_t type_by_typename(std::string name);
    static std::string typename_by_type(type_t type);


    static inline bool is_number(type_t t) noexcept {
        return t == type_t::INTEGER
            || t == type_t::DOUBLE || t == type_t::BOOLEAN;
    }


    static inline bool is_float(type_t t) noexcept {
        return t == type_t::DOUBLE;
    }


    static inline bool is_integral(type_t t) noexcept {
        return t == type_t::INTEGER || t == type_t::BOOLEAN;
    }

    static inline bool is_string(type_t t) noexcept {
       return t == type_t::STRING;
    }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_VARIABLE_H__
