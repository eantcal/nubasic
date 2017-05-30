//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_token.h"
#include "nu_cpp_lang.h"


/* -------------------------------------------------------------------------- */

std::string nu::token_t::description_of(tkncl_t tc)
{
    std::string desc;

    switch (tc) {
    case nu::tkncl_t::BLANK:
        desc = "BLNK";
        break;

    case nu::tkncl_t::INTEGRAL:
        desc = "INTG";
        break;

    case nu::tkncl_t::NEWLINE:
        desc = "NEWL";
        break;

    case nu::tkncl_t::REAL:
        desc = "REAL";
        break;

    case nu::tkncl_t::OPERATOR:
        desc = "OPER";
        break;

    case nu::tkncl_t::SUBEXP_BEGIN:
        desc = "BGNE";
        break;

    case nu::tkncl_t::SUBEXP_END:
        desc = "ENDE";
        break;

    case nu::tkncl_t::STRING_LITERAL:
        desc = "STRL";
        break;

    case nu::tkncl_t::STRING_COMMENT:
        desc = "COMM";
        break;

    case nu::tkncl_t::IDENTIFIER:
        desc = "IDNT";
        break;

    case nu::tkncl_t::UNDEFINED:
    default:
        desc = "UNDF";
        break;
    }

    return desc;
}


/* -------------------------------------------------------------------------- */

void nu::token_t::set_identifier(const std::string& id, case_t casemode)
{
    _identifier = id;
    _org_id = id;

    switch (casemode) {
    case case_t::LOWER:
        _set_id_lowercase();
        break;

    case case_t::NOCHANGE:
    default:
        // do nothing
        break;
    }
}


/* -------------------------------------------------------------------------- */

nu::token_t::token_t(const std::string& id, tkncl_t t, size_t pos,
    data_ptr_t expr_ptr) noexcept : _identifier(id),
                                    _org_id(id),
                                    _type(t),
                                    _position(pos),
                                    _expression_ptr(expr_ptr)
{
    _set_id_lowercase();
}
