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
#include <cassert>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

std::u16string utf8_to_u16(const std::string& s)
{
    std::u16string result;
    for (size_t i = 0; i < s.size(); ) {
        uint32_t cp = 0;
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t extra = 0;
        if (c < 0x80)        { cp = c;        extra = 0; }
        else if (c < 0xC0)   { ++i; continue; }           // invalid continuation byte
        else if (c < 0xE0)   { cp = c & 0x1F; extra = 1; }
        else if (c < 0xF0)   { cp = c & 0x0F; extra = 2; }
        else                 { cp = c & 0x07; extra = 3; }
        ++i;
        for (size_t j = 0; j < extra && i < s.size(); ++j, ++i)
            cp = (cp << 6) | (static_cast<unsigned char>(s[i]) & 0x3F);
        if (cp < 0x10000) {
            result += static_cast<char16_t>(cp);
        } else {
            cp -= 0x10000;
            result += static_cast<char16_t>(0xD800u + (cp >> 10));
            result += static_cast<char16_t>(0xDC00u + (cp & 0x3FFu));
        }
    }
    return result;
}


/* -------------------------------------------------------------------------- */

std::string u16_to_utf8(const std::u16string& s)
{
    std::string result;
    for (size_t i = 0; i < s.size(); ) {
        uint32_t cp;
        char16_t c = s[i++];
        if (c >= 0xD800u && c <= 0xDBFFu && i < s.size()) {
            char16_t low = s[i];
            if (low >= 0xDC00u && low <= 0xDFFFu) {
                cp = 0x10000u + ((static_cast<uint32_t>(c) - 0xD800u) << 10)
                              + (static_cast<uint32_t>(low) - 0xDC00u);
                ++i;
            } else {
                cp = c;
            }
        } else {
            cp = c;
        }
        if (cp < 0x80) {
            result += static_cast<char>(cp);
        } else if (cp < 0x800) {
            result += static_cast<char>(0xC0u | (cp >> 6));
            result += static_cast<char>(0x80u | (cp & 0x3Fu));
        } else if (cp < 0x10000) {
            result += static_cast<char>(0xE0u | (cp >> 12));
            result += static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
            result += static_cast<char>(0x80u | (cp & 0x3Fu));
        } else {
            result += static_cast<char>(0xF0u | (cp >> 18));
            result += static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu));
            result += static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
            result += static_cast<char>(0x80u | (cp & 0x3Fu));
        }
    }
    return result;
}


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
      return utf8_to_u16(s);
   };

   for (size_t i = 0; i < unescaped.size(); ++i) {

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


