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

#ifndef __NU_EXPR_PARSER_H__
#define __NU_EXPR_PARSER_H__


/* -------------------------------------------------------------------------- */

#include "nu_var_scope.h"
#include "nu_proc_scope.h"
#include "nu_expr_tknzr.h"
#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_token_list.h"

#include <list>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class expr_parser_t
{
public:
   //! ctors
   expr_parser_t() = default;
   expr_parser_t(const expr_parser_t&) = delete;
   expr_parser_t& operator=( const expr_parser_t& ) = delete;

   //! Creates an expression using tokens got by a given tokenizer
   expr_any_t::handle_t compile(expr_tknzr_t& tknzr);

   //! Creates an expression using a given token-list
   expr_any_t::handle_t compile(token_list_t tl, size_t expr_pos);


protected:
   static variant_t::type_t get_type(const token_t& t);

   expr_any_t::handle_t parse_operand(token_list_t& tl);

   bool parse_operator(token_list_t& tl, std::string & value);

   expr_any_t::handle_t parse(
      token_list_t& tl,
      expr_any_t::handle_t first_param = nullptr);

   static void reduce_brackets(token_list_t& rtl);
   static void convert_subscription_brackets(token_list_t& rtl);

   static void fix_real_numbers(token_list_t& rtl);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_PARSER_H__
