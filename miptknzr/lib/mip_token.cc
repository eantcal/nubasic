//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "mip_token.h"


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

const char_t* token_t::type2str(tcl_t type) {
    switch (type) {
    case tcl_t::ATOM:
        return _T("atomic");
    case tcl_t::BLANK:
        return _T("blank");
    case tcl_t::COMMENT:
        return _T("comment");
    case tcl_t::END_OF_FILE:
        return _T("eof");
    case tcl_t::END_OF_LINE:
        return _T("eol");
    case tcl_t::OTHER:
        return _T("other");
    case tcl_t::STRING:
        return _T("string");
    default:
        break;
    }

    return _T("unknown");
}


/* -------------------------------------------------------------------------- */

_ostream& operator<<(_ostream& os, token_t& tkn) {
    os << _T("type:'")
        << token_t::type2str(tkn.type())
        << _T("' value:'")
        << tkn.value() << _T("' at ")
        << tkn.line() + 1 << _T(".") << tkn.offset() + 1
        << std::endl;

    return os;
}


/* -------------------------------------------------------------------------- */

}
