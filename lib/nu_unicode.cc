//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "nu_unicode.h"
#include <string>
#include <codecvt>
#include <cassert>
#include <locale>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

std::u16string unicode_unescape(const std::string& unescaped)
{
   enum class fsm_st_t {
      IDLE,
      WF_ESCAPE,
      WF_HEX
   };

   fsm_st_t st = fsm_st_t::IDLE;

   std::string tmp;
   std::u16string res;

   auto isValid = [](char c) {
      return (c >= 'A' && c <= 'F') || 
             (c >= 'a' && c <= 'f') || 
             (c >= '0' && c <= '9');
   };


   auto convertToU16 = [](const std::string& s) {
      return std::wstring_convert<
         std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(s);
   };

   for (int i = 0; i < unescaped.size(); ++i) {

      auto ch = unescaped[i];

      if (st == fsm_st_t::IDLE) {
         if (ch == '\\') {
            st = fsm_st_t::WF_ESCAPE;
            tmp += unescaped[i];
         }
         else {
            res += convertToU16(tmp);
            tmp.clear();

            res += ch;
         }
      }
      else if (st == fsm_st_t::WF_ESCAPE) {
         if (ch == 'u' || ch == 'U') {
            tmp += ch;
            st = fsm_st_t::WF_HEX;
         }
         else {
            res += convertToU16(tmp);
            tmp.clear();

            res += ch;

            st = fsm_st_t::IDLE;
         }
      }
      else if (st == fsm_st_t::WF_HEX) {
         if (!isValid(ch)) {
            res += convertToU16(tmp);
            tmp.clear();

            res += ch;

            st = fsm_st_t::IDLE;
         }
         else {
            tmp += ch;

            // tmp is something like "\\u1fa3" (length(tmp)==6)
            if (tmp.size() >= 6) { 
               std::stringstream ss;

               uint16_t ch16 = 0;
               ss << tmp.c_str() + 2;
               tmp.clear();
               
               ss >> std::hex >> ch16;
               
               res += ch16;
               
               st = fsm_st_t::IDLE;
            }
         }
      }
   }

   if (!tmp.empty()) {
      res += convertToU16(tmp);
   }

   return res;
}


/* -------------------------------------------------------------------------- */

// namespace nu

}


