//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_TOKENIZER_H__
#define __NU_TOKENIZER_H__


/* -------------------------------------------------------------------------- */

#include "nu_basic_defs.h"
#include "nu_expr_tknzr.h"

#include <cstdio>
#include <map>
#include <string.h>
#include <string>

namespace nu {

/* ------------------------------------------------------------------------- */

struct tokenizer_t : public expr_tknzr_t 
{
    tokenizer_t(const std::string& data, size_t pos = 0)
        : expr_tknzr_t(data, pos, NU_BASIC_BLANKS, NU_BASIC_NEWLINES,
              NU_BASIC_SINGLE_CHAR_OPS, 
              NU_BASIC_WORD_OPS,
              NU_BASIC_BEGIN_SUBEXPR, 
              NU_BASIC_END_SUBEXPR,
              NU_BASIC_BEGIN_STRING, 
              NU_BASIC_END_STRING, 
              NU_BASIC_ESCAPE_CHAR,
              NU_BASIC_LINE_COMMENT
             )
    {
    }
};


/* ------------------------------------------------------------------------- */

}


/* ------------------------------------------------------------------------- */

#endif // NU_TOKENIZER
