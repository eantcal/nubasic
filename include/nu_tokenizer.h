
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

struct tokenizer_t : public expr_tknzr_t {
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
}

#endif // NU_TOKENIZER
