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

#ifndef __NU_EXPR_SYNTAX_TREE_H__
#define __NU_EXPR_SYNTAX_TREE_H__


/* -------------------------------------------------------------------------- */

#include "nu_variable.h"
#include "nu_var_scope.h"
#include "nu_expr_parser.h"
#include "nu_exception.h"

#include <string>
#include <memory>
#include <iostream>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

/**
* Transform a given token list in its intermediate representation for
* the expression parser. The token list is transformed in a abstract syntax
* tree where each node denotes a "well-formed" expression.
*/
struct expr_syntax_tree_t
{
   //! ctors
   expr_syntax_tree_t();
   expr_syntax_tree_t(expr_syntax_tree_t&) = default;
   expr_syntax_tree_t& operator=( expr_syntax_tree_t& ) = default;

protected:
   std::string explicit_left_marker = "(";
   std::string explicit_right_marker = ")";
   std::string implicit_left_marker = "";
   std::string implicit_right_marker = "";
   std::set<std::string> processing_ops;
   std::set<std::string> ignoring_ops;

   enum class op_preced_t
   {
      MATH_MUL_DIV,
      MATH_SUM,
      LOGICAL_BINOP,
      LOGICAL_ANDOR
   };

   void change_operator_precedence(op_preced_t precedence);
   void swap_lr_markers();

   token_list_t remove_blank(token_list_t tl);
   void rework_subexp(token_list_t & tl, token_list_t::data_t::iterator i);
   void rework_operator(token_list_t & tl, token_list_t::data_t::iterator i);
   void skip_processed(token_list_t & tl, token_list_t::data_t::iterator& i);
   void rework_minus_operator(token_list_t & tl, token_list_t::data_t::iterator i);
   void rework_unary_operator(token_list_t & tl, token_list_t::data_t::iterator i);
   void rework_identifier(token_list_t & tl, token_list_t::data_t::iterator i);
   token_list_t rework_token_list(token_list_t tl);


public:
   //! Converts a given token list into syntax-oriented
   //! and well-formed token list
   token_list_t operator()(token_list_t tl);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_EXPR_SYNTAX_TREE_H__
