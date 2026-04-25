//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_reserved_keywords.h"
#include "nu_keyword_categories.h"

/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

std::set<std::string>& reserved_keywords_t::list()
{
    static std::set<std::string> list = [] {
        std::set<std::string> result;
#define NUBASIC_INSERT_KEYWORD(keyword_literal) result.insert(keyword_literal);
        NUBASIC_KEYWORDS_ALL(NUBASIC_INSERT_KEYWORD)
#undef NUBASIC_INSERT_KEYWORD
        return result;
    }();

    return list;
}

const std::vector<std::string>& reserved_keywords_t::graphics()
{
    static const std::vector<std::string> list = {
#define NUBASIC_GRAPHICS_KEYWORD(keyword_literal) keyword_literal,
        NUBASIC_KEYWORDS_GRAPHICS(NUBASIC_GRAPHICS_KEYWORD)
#undef NUBASIC_GRAPHICS_KEYWORD
    };

    return list;
}

/* -------------------------------------------------------------------------- */

} // namespace nu

/* -------------------------------------------------------------------------- */
