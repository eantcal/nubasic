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

#include "nu_basic_defs.h"
#include "nu_expr_parser.h"
#include "nu_expr_syntax_tree.h"
#include "nu_expr_empty.h"
#include "nu_expr_var.h"
#include "nu_expr_bin.h"
#include "nu_expr_literal.h"
#include "nu_expr_function.h"
#include "nu_expr_subscrop.h"
#include "nu_expr_unary_op.h"
#include "nu_variant.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

expr_any_t::handle_t expr_parser_t::compile(expr_tknzr_t& tknzr)
{
   token_list_t tl;

   //Split expression in tokens
   tknzr.get_tknlst(tl);

   //Create a syntax tree
   expr_syntax_tree_t est;

   //Finally parse the expression tree in order to generate
   //an executable object
   convert_subscription_brackets(tl);
   tl = est(tl);
   return parse(tl);
}


/* -------------------------------------------------------------------------- */

expr_any_t::handle_t expr_parser_t::compile(token_list_t tl, size_t expr_pos)
{
   expr_syntax_tree_t est;
   convert_subscription_brackets(tl);
   tl = est(tl);
   return parse(tl);
}


/* -------------------------------------------------------------------------- */

variant_t::type_t expr_parser_t::get_type(const token_t& t)
{
   switch (t.type())
   {
      case tkncl_t::INTEGRAL:
         return variant_t::type_t::LONG64;

      case tkncl_t::REAL:
         return variant_t::type_t::DOUBLE;

      case tkncl_t::STRING_LITERAL:
         return variant_t::type_t::STRING;

      default:
         break;
   }

   if (t.type() == tkncl_t::IDENTIFIER &&
         (t.identifier() == "true" ||
          t.identifier() == "false"))
   {
      return variant_t::type_t::BOOLEAN;
   }

   return variant_t::type_t::UNDEFINED;
}


/* -------------------------------------------------------------------------- */

expr_any_t::handle_t expr_parser_t::parse_operand(token_list_t& tl)
{
   //An empty list generates an empty expression
   if (tl.empty())
      return expr_any_t::handle_t(std::make_shared< expr_empty_t>());

   reduce_brackets(tl);

   token_t t(*tl.begin());

   switch (t.type())
   {
      case tkncl_t::OPERATOR:

         // ++<identifier> / --<identifier>
         if (tl.size() > 1 &&
               (tl[0].identifier() == NU_BASIC_OP_INC ||
                tl[0].identifier() == NU_BASIC_OP_DEC) &&
               tl[1].type() == tkncl_t::IDENTIFIER)
         {
            auto ret_handle =
               std::make_shared<expr_unary_op_t>(
                  tl[0].identifier(),
                  std::make_shared<expr_var_t>(tl[1].identifier()));

            tl.data().erase(tl.begin(), tl.begin() + 2);

            return tl.empty() ? ret_handle : parse(tl, ret_handle);
         }

         break;

      case tkncl_t::IDENTIFIER:

         // <identifier>+"(" => function
         if (tl.size() > 1 && tl[1].type() == tkncl_t::SUBEXP_BEGIN)
         {
            std::string function_name = tl.begin()->identifier();
            func_args_t function_args;

            size_t size = 0;

            if (tl.size() > 2 && tl[2].type() == tkncl_t::SUBEXP_END)
            {
               size = 3;
            }
            else
            {
               // Splits the parameters list in listes of tokens
               auto params = tl.get_parameters(
               { NU_BASIC_BEGIN_SUBEXPR_OP, tkncl_t::SUBEXP_BEGIN },
               { NU_BASIC_END_SUBEXPR_OP, tkncl_t::SUBEXP_END },
               { NU_BASIC_PARAM_SEP, tkncl_t::OPERATOR });

               //size here starts from 2.
               //One is identifier and one is "("
               size = 2;

               //Parse each parameter
               for (auto par : params)
               {
                  auto param_tl = par;
                  size += par.size() + 1;

                  reduce_brackets(param_tl);

                  expr_any_t::handle_t arg(parse(param_tl));
                  function_args.push_back(arg);
               }
            }

            // Remove from original list token parsed
            tl.data().erase(tl.begin(), tl.begin() + size);

            expr_any_t::handle_t ret_handle = nullptr;

            if (function_name.size()>1 &&
                  *function_name.rbegin() == NU_BASIC_BEGIN_SUBSCR)
            {
               ret_handle =
                  std::make_shared<expr_subscrop_t>(
                     function_name.substr(0, function_name.size() - 1),
                     function_args);

            }
            else
            {
               // Create an executable object for the parsed function
               ret_handle =
                  std::make_shared<expr_function_t>(function_name, function_args);
            }

            // Check if we have other to parse
            // YES: call recursively parse()
            // NO : return the handle to executable object
            return tl.empty() ? ret_handle : parse(tl, ret_handle);
         }
         else
         {
            std::string id = t.identifier();
            expr_any_t::handle_t ret_handle;

            if (id == "true")
            {
               tl.data().erase(tl.begin());

               ret_handle =
                  expr_any_t::handle_t(
                     std::make_shared<expr_literal_t>(variant_t(true)));
            }
            else if (id == "false")
            {
               tl.data().erase(tl.begin());

               ret_handle =
                  expr_any_t::handle_t(
                     std::make_shared<expr_literal_t>(variant_t(false)));
            }
            // 0xnnnnnn  (hexadecimal value)
            else if (id.size() > 2 &&
                     id.c_str()[0] == '&' && toupper(id.c_str()[1]) == 'H')
            {
               tl.data().erase(tl.begin());

               std::string hex = id.substr(2, id.size() - 2);
               int n = 0;
               sscanf(hex.c_str(), "%x", &n);

               ret_handle =
                  expr_any_t::handle_t(
                     std::make_shared<expr_literal_t>(variant_t(n)));
            }
            else
            {
               std::string id = t.identifier();

               if (id.size() > 1 && *id.rbegin() == NU_BASIC_BEGIN_SUBSCR)
                  id = id.substr(0, id.size() - 1);

               if (!variable_t::is_valid_name(id, false))
               {
                  syntax_error(t.expression(), t.position(),
                               "\"" + id + "\" is an invalid identifier");
               }

               tl.data().erase(tl.begin());

               ret_handle = expr_any_t::handle_t(
                               std::make_shared<expr_var_t>(id));
            }

            return tl.empty() ? ret_handle : parse(tl, ret_handle);
         }

         break;


      // numerical token
      case tkncl_t::INTEGRAL:
      case tkncl_t::REAL:
      case tkncl_t::STRING_LITERAL:
         do
         {
            //Remove token from tl
            tl.data().erase(tl.begin());

            // Generates a literal using a "variant" instance
            // for the executable object
            expr_any_t::handle_t ret_handle(
               std::make_shared<expr_literal_t>(
                  variant_t(t.identifier(), get_type(t))));

            // Check if we have other to parse
            // YES: call recursively parse()
            // NO : return the handle to executable object
            return tl.empty() ? ret_handle : parse(tl, ret_handle);
         }
         while (0);

      case tkncl_t::SUBEXP_BEGIN:

         // this is a sub expression
         if (tl.size() >= 3)
         {
            token_list_t rtl =
               tl.sublist(tkncl_t::SUBEXP_BEGIN, tkncl_t::SUBEXP_END, 0, false);

            tl.data().erase(tl.begin(), tl.begin() + rtl.size());

            //simplify "(((....)))"
            reduce_brackets(rtl);

            //Parse the sub-expression
            auto h(parse(rtl));

            // Check if we have other to parse
            // YES: call recursively parse()
            // NO : return the handle to executable object
            return tl.empty() ? h : parse(tl, h);
         }
         else
         {
            return expr_any_t::handle_t(std::make_shared<expr_empty_t>());
         }

         break;

      default:
         break;

   }

   return expr_any_t::handle_t(std::make_shared<expr_empty_t>());
}


/* -------------------------------------------------------------------------- */

bool expr_parser_t::parse_operator(token_list_t& tl, std::string& value)
{
   if (tl.empty())
      return false;

   token_t t(*tl.begin());

   if (t.type() != tkncl_t::OPERATOR)
      return false;

   value = t.identifier();
   tl.data().erase(tl.begin());

   return true;
}


/* -------------------------------------------------------------------------- */

expr_any_t::handle_t expr_parser_t::parse(
   token_list_t & tl,
   expr_any_t::handle_t first_param)
{
   fix_real_numbers(tl);

   if (!first_param)
      first_param = parse_operand(tl);

   if (!first_param)
      return first_param;

   if (tl.empty())
      return first_param;

   reduce_brackets(tl);

   std::string op;
   expr_any_t::handle_t second_param;

   syntax_error_if(!parse_operator(tl, op), NU_BASIC_ERROR_STR__SYNTAXERROR);
   second_param = parse_operand(tl);
   syntax_error_if(!second_param, NU_BASIC_ERROR_STR__SYNTAXERROR);

   // resolve built-in operator implementation
   auto operator_implementation = global_operator_tbl_t::get_instance()[op];

   // create the expression object using built-in
   // operator_implementation semantic
   return expr_any_t::handle_t(
             std::make_shared<expr_bin_t>(operator_implementation, first_param, second_param));
}


/* -------------------------------------------------------------------------- */

void expr_parser_t::fix_real_numbers(token_list_t & rtl)
{
   auto tl_size = rtl.size();

   if (tl_size < 3)
      return;

   token_list_t tl(std::move(rtl));
   rtl.clear();

   for (decltype(tl_size) i = 0; i < tl_size; ++i)
   {
      auto tk = tl[i];

      if (tk.type() == tkncl_t::REAL)
      {
         const std::string& id = tk.identifier();

         if (!id.empty() &&
               ::toupper(id.c_str()[id.size() - 1]) == NU_BASIC_EXPONENT_SYMB &&
               i < (tl_size - 1))
         {
            const auto & tkop = tl[i + 1];

            if (tkop.type() == tkncl_t::OPERATOR &&
                  (tkop.identifier() == "+" || tkop.identifier() == "-") &&
                  i < (tl_size - 2))
            {
               const auto & tkn = tl[i + 2];

               if (tkn.type() == tkncl_t::INTEGRAL)
               {
                  std::string new_id =
                     tk.identifier() +
                     tkop.identifier() +
                     tkn.identifier();

                  tk.set_identifier(new_id, nu::token_t::case_t::LOWER);

                  i += 2;
               }
            }
         }


      }

      rtl += tk;
   }

}


/* -------------------------------------------------------------------------- */

void expr_parser_t::reduce_brackets(token_list_t& rtl)
{
   //simplify "(((....)))"
   while (rtl.size() > 2 &&
          rtl.begin()->type() == tkncl_t::SUBEXP_BEGIN &&
          (rtl.end() - 1)->type() == tkncl_t::SUBEXP_END)
   {
      auto substr =
         rtl.sublist(
            tkncl_t::SUBEXP_BEGIN, tkncl_t::SUBEXP_END, 0, false);

      if (substr.size() == (rtl.size()))
      {
         --rtl; //remove head token
         rtl--; //remove tail token
      }
      else
      {
         break;
      }
   }
}


/* -------------------------------------------------------------------------- */

void expr_parser_t::convert_subscription_brackets(token_list_t& rtl)
{
   token_t * token_prev = nullptr;

   for (size_t i = 0; i < rtl.size(); ++i)
   {
      auto & token = rtl[i];

      if (i > 0)
         token_prev = &rtl[i - 1];

      if (token.type() == tkncl_t::SUBSCR_BEGIN)
      {
         if (token_prev)
         {
            auto id = token_prev->identifier();

            if (!id.empty() && *id.rbegin() != NU_BASIC_BEGIN_SUBSCR)
            {
               id.push_back(NU_BASIC_BEGIN_SUBSCR);
               token_prev->set_identifier(id, nu::token_t::case_t::LOWER);
            }
         }

         token.set_type(tkncl_t::SUBEXP_BEGIN);

         if (token.identifier() == NU_BASIC_BEGIN_SUBSCR_OP)
            token.set_identifier(NU_BASIC_BEGIN_SUBEXPR_OP, nu::token_t::case_t::LOWER);
      }

      else if (token.type() == tkncl_t::SUBSCR_END)
      {
         token.set_type(tkncl_t::SUBEXP_END);

         if (token.identifier() == NU_BASIC_END_SUBSCR_OP)
            token.set_identifier(NU_BASIC_END_SUBEXPR_OP, nu::token_t::case_t::LOWER);
      }
   }
}


/* -------------------------------------------------------------------------- */

} //namespace


/* -------------------------------------------------------------------------- */

