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
*  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
*
*/

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_TKNZR_H__
#define __NU_EXPR_TKNZR_H__


/* -------------------------------------------------------------------------- */

#include "nu_base_tknzr.h"

#include "nu_icstring.h"
#include "nu_lxa.h"
#include "nu_token.h"
#include "nu_token_list.h"

#include <deque>
#include <set>
#include <ostream>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

/**
 * This class implements a tokenizer which provides a container view interface
 * of a series of tokens contained in a given expression string
 */
class expr_tknzr_t : public base_tknzr_t
{
public:
   expr_tknzr_t() = delete;
   expr_tknzr_t(const expr_tknzr_t&) = delete;
   expr_tknzr_t& operator=( const expr_tknzr_t& ) = delete;
   virtual ~expr_tknzr_t() {}

   expr_tknzr_t(
      const std::string& data, //expression string data
      size_t pos, // original position of expression
      // used only to generate syntax error info
      // Each token position marker is the result of the sum
      // between this value and the token position
      // in the expression
      const std::string& blanks,    //Blanks symbols
      const std::string& newlines,  //New line symbols
      const std::string& operators, //Single char operators
      const std::set<std::string>& str_op, //Word operators
      const char subexp_bsymb,   //Begin sub-expression symbol
      const char subexp_esymb,   //End sub-expression symbol
      const std::string& string_bsymb, // Begin string marker
      const std::string& string_esymb, // End string marker
      const char string_escape         // Escape string symbol
   );


   //! Get a token and advance to the next one (if any)
   virtual token_t next() override;


   //! Split expression in token and copy them into a token list
   virtual void get_tknlst(token_list_t& tl) override;


   //! Get begin sub-expression symbol
   std::string subexp_begin_symbol() const NU_NOEXCEPT
   {
      return _subexp_begin_symb;
   }

   //! Get end sub-expression symbol
   std::string subexp_end_symbol() const NU_NOEXCEPT
   {
      return _subexp_end_symb;
   }


   //! Return expression position in the source line
   size_t get_exp_pos() const NU_NOEXCEPT
   {
      return _pos;
   }


protected:
   size_t _pos = 0;
   std::string _subexp_begin_symb;
   std::string _subexp_end_symb;
   std::set<std::string> _str_op; //mod, div, ...
   lxa_symb_t _blank, _op, _newl;
   lxa_word_t _word_op;
   lxa_str_t _strtk;

};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_TKNZR_H__
