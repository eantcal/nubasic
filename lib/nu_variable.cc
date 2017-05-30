//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_variable.h"
#include "nu_exception.h"

#include <algorithm>
#include <cassert>
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variable_t::type_t variable_t::type_by_typename(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), tolower);

    if (name == "integer")
        return type_t::INTEGER;

    if (name == "long64")
        return type_t::LONG64;

    if (name == "string")
        return type_t::STRING;

    if (name == "bytevect")
        return type_t::BYTEVECTOR;

    if (name == "double")
        return type_t::DOUBLE;

    if (name == "float")
        return type_t::FLOAT;

    if (name == "boolean")
        return type_t::BOOLEAN;

    if (name == "any")
        return type_t::ANY;

    return type_t::UNDEFINED;
}


/* -------------------------------------------------------------------------- */

std::string variable_t::typename_by_type(variable_t::type_t type)
{
    switch (type) {
    case type_t::INTEGER:
        return "integer";

    case type_t::STRING:
        return "string";

    case type_t::BOOLEAN:
        return "boolean";

    case type_t::FLOAT:
        return "float";

    case type_t::LONG64:
        return "long64";

    case type_t::BYTEVECTOR:
        return "bytevect";

    case type_t::DOUBLE:
        return "double";

    case type_t::STRUCT:
        return "struct";

    case type_t::ANY:
        return "any";

    case type_t::UNDEFINED:
        break;
    }

    return "Undefined";
}


/* -------------------------------------------------------------------------- */

variable_t::type_t variable_t::type_by_name(const std::string& name)
{
    assert(name.size() > 0);

    const char ch = *name.rbegin();

    size_t pos = name.find_first_of('.', 0);

    if (pos != size_t(-1))
        return type_t::STRUCT;

    switch (ch) {
    case '%':
        return type_t::INTEGER;

    case '$':
        return type_t::STRING;

    case '@':
        return type_t::BYTEVECTOR;

    case '!':
        return type_t::DOUBLE;

    case '&':
        return type_t::LONG64;

    case '#':
        return type_t::BOOLEAN;
    }

    return type_t::FLOAT;
}


/* -------------------------------------------------------------------------- */

bool variable_t::is_valid_name(std::string name, bool ingnore_builtin)
{
    if (name.empty())
        return false;

    std::transform(name.begin(), name.end(), name.begin(), tolower);

    if (!ingnore_builtin
        && reserved_keywords_t::list.find(name)
            != reserved_keywords_t::list.cend())
        return false;

    auto letter = [](char c) { return c >= 'a' && c <= 'z'; };

    char first_char = name.c_str()[0];

    if (!letter(first_char) && first_char != '_')
        return false;

    if (name.size() == 1)
        return first_char != '_';

    auto number = [](char c) { return (c >= '0' && c <= '9'); };

    char prev_c = 0;

    for (size_t i = 1; i < name.size(); ++i) {
        char c = name.c_str()[i];

        bool valid = letter(c) || (c == '.' && prev_c != '.') || c == '_'
            || number(c) || (i == (name.size() - 1)
                                && (c == '%' || c == '#' || c == '$' || c == '@'
                                       || c == '!' || c == '&'));

        if (!valid)
            return false;

        prev_c = c;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
