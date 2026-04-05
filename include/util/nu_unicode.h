//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_UNICODE_H
#define __NU_UNICODE_H


/* -------------------------------------------------------------------------- */

#include "nu_basic_defs.h"
#include "nu_expr_tknzr.h"


/* -------------------------------------------------------------------------- */

namespace nu {
   //! Returns the u16 string converting any escaped sequence of
   //! \\uHHHH or \\UHHHH in corrisponding Unicode16 character.
   //! HHHH must be an hexadecimal string of exactly 4 characters (0000 - FFFF)
   //! \\u represents the string "\u" and \\U represents "\U"
   //! Any escaped sequence invalid according the previous rule will be
   //! assumed as a plain string
   std::u16string unicode_unescape(const std::string& unescaped);

   //! Converts a UTF-8 encoded std::string to std::u16string (C++20, no <codecvt>)
   std::u16string utf8_to_u16(const std::string& s);

   //! Converts a std::u16string to a UTF-8 encoded std::string (C++20, no <codecvt>)
   std::string u16_to_utf8(const std::u16string& s);
}


/* -------------------------------------------------------------------------- */

#endif // __NU_UNICODE_H
