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
#include "nu_global_function_tbl.h"
#include "nu_token_list.h"
#include "nu_label_tbl.h"

#include "nu_stmt_beep.h"
#include "nu_stmt_block.h"
#include "nu_stmt_cls.h"
#include "nu_stmt_close.h"
#include "nu_stmt_call.h"
#include "nu_stmt_delay.h"
#include "nu_stmt_const.h"
#include "nu_stmt_dim.h"
#include "nu_stmt_redim.h"
#include "nu_stmt_end.h"
#include "nu_stmt_endif.h"
#include "nu_stmt_endsub.h"
#include "nu_stmt_endfunction.h"
#include "nu_stmt_else.h"
#include "nu_stmt_exit_do.h"
#include "nu_stmt_exit_for.h"
#include "nu_stmt_exit_while.h"
#include "nu_stmt_exit_sub.h"
#include "nu_stmt_expr.h"
#include "nu_stmt_for_to_step.h"
#include "nu_stmt_gosub.h"
#include "nu_stmt_goto.h"
#include "nu_stmt_on_goto.h"
#include "nu_stmt_if_then_else.h"
#include "nu_stmt_input.h"
#include "nu_stmt_input_file.h"
#include "nu_stmt_let.h"
#include "nu_stmt_locate.h"
#include "nu_stmt_next.h"
#include "nu_stmt_open.h"
#include "nu_stmt_print.h"
#include "nu_stmt_randomize.h"
#include "nu_stmt_read.h"
#include "nu_stmt_return.h"
#include "nu_stmt_do.h"
#include "nu_stmt_loop_while.h"
#include "nu_stmt_while.h"
#include "nu_stmt_wend.h"
#include "nu_stmt_sub.h"
#include "nu_stmt_function.h"
#include "nu_stmt_struct.h"
#include "nu_stmt_endstruct.h"
#include "nu_stmt_struct_element.h"


#include "nu_stmt_parser.h"
#include "nu_expr_parser.h"
#include "nu_expr_tknzr.h"

#include "nu_stmt_geninstr.h"
#include "nu_stdlib.h"

#ifndef TINY_NUBASIC_VER
#include "nu_os_gdi.h"
#endif

#include <set>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

using stmt_shell_t = stmt_geninstr_t<os_shell_t, variant_t, 1>;
using stmt_chdir_t = stmt_geninstr_t<os_chdir_t, variant_t, 1>;

using stmt_fopen_t = stmt_geninstr_t<os_fopen_t, variant_t, 3>;
using stmt_fseek_t = stmt_geninstr_t<os_fseek_t, variant_t, 3>;
using stmt_flush_t = stmt_geninstr_t<os_fflush_t, variant_t, 1>;

#ifndef TINY_NUBASIC_VER
using stmt_textout_t = stmt_geninstr_t<os_textout_t, gdi_varg_t, 4>;
using stmt_plotimage_t = stmt_geninstr_t<os_plotimage_t, gdi_varg_t, 3>;
using stmt_line_t = stmt_geninstr_t<os_line_t, gdi_iarg_t, 5>;
using stmt_set_pixel_t = stmt_geninstr_t<os_set_pixel_t, gdi_iarg_t, 3>;
using stmt_rect_t = stmt_geninstr_t<os_rect_t, gdi_iarg_t, 5>;
using stmt_fillrect_t = stmt_geninstr_t<os_fillrect_t, gdi_iarg_t, 5>;
using stmt_ellipse_t = stmt_geninstr_t<os_ellipse_t, gdi_iarg_t, 5>;
using stmt_fillellipse_t = stmt_geninstr_t<os_fillellipse_t, gdi_iarg_t, 5>;
#endif // TINY_NUBASIC_VER


/* -------------------------------------------------------------------------- */

void stmt_parser_t::remove_blank(nu::token_list_t & tl)
{
   while (!tl.empty() && (
             tl.begin()->type() == tkncl_t::BLANK ||
             tl.begin()->type() == tkncl_t::LINE_COMMENT ||
             tl.begin()->type() == tkncl_t::NEWLINE))
   {
      --tl;
   }

   while (!tl.empty() &&
          (tl.rbegin()->type() == tkncl_t::BLANK ||
           tl.begin()->type() == tkncl_t::LINE_COMMENT))
   {
      tl--;
   }
}


/* -------------------------------------------------------------------------- */

void stmt_parser_t::extract_next_token(
   token_list_t& tl,
   token_t & token,
   std::function<bool(const token_list_t& tl, const token_t& token)> check)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(check(tl, token), token.expression(), token.position());
   token = *tl.begin();
}


/* -------------------------------------------------------------------------- */

void stmt_parser_t::move_sub_expression(
   token_list_t& source_tl,
   token_list_t& dst_tl,
   const std::string& id,
   tkncl_t idtype)
{
   while (!source_tl.empty() &&
          (!(source_tl.begin()->type() == idtype &&
             source_tl.begin()->identifier() == id)))
   {
      const token_t token(*source_tl.begin());
      dst_tl += token;
      --source_tl;
      remove_blank(source_tl);
   }
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_print(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   int fd = 0;

   //print# filenum
   if (token.identifier() == "print#" || token.identifier() == "write#")
   {
      token = *tl.begin();

      syntax_error_if(token.type() != tkncl_t::INTEGRAL,
                      token.expression(),
                      token.position());

      try
      {
         fd = nu::stoi(token.identifier());
      }
      catch (...)
      {
         syntax_error(token.expression(), token.position());
      }

      --tl;
      remove_blank(tl);

      if (!tl.empty()) // must be a comma separator
      {
         token = *tl.begin();
         syntax_error_if(
            token.type() != tkncl_t::OPERATOR ||
            token.identifier() != ",",
            token.expression(),
            token.position());
         --tl; // remove comma from tl
      }
   }

   //print #filenum
   else if (! tl.empty())
   {
      token = *tl.begin();

      if (token.type() == tkncl_t::IDENTIFIER)
      {
         std::string id = token.identifier();

         if (id.size() > 1 && id.c_str()[0] == '#')
         {
            try
            {
               fd = nu::stoi(id.substr(1, id.size() - 1));
            }
            catch (...)
            {
               syntax_error(token.expression(), token.position());
            }

            --tl;
            remove_blank(tl);

            if (!tl.empty())
            {
               token = *tl.begin();

               syntax_error_if(
                  token.type() != tkncl_t::OPERATOR ||
                  token.identifier() != ",",
                  token.expression(),
                  token.position());
               --tl; // remove comma from tl
            }

         }
      }
   }

   syntax_error_if(fd<0, token.expression(), token.position());

   if (tl.empty())
   {
      return stmt_t::handle_t(std::make_shared<stmt_print_t>(ctx, fd));
   }


   if (token.type() == tkncl_t::STRING_LITERAL && tl.size() == 1)
   {
      --tl;

      //Create statement to print out a literal string
      return stmt_t::handle_t(
                std::make_shared<stmt_print_t>(ctx, fd, token.identifier()));
   }

   return parse_arg_list<stmt_print_t, 0>(
      ctx,
      token,
      tl,
      [](const token_t&t) {
         return t.type()==tkncl_t::OPERATOR &&
                  (t.identifier() == "," || t.identifier() == ";");
      },
      ctx,
      fd
   );
}


/* -------------------------------------------------------------------------- */

var_arg_t stmt_parser_t::parse_var_arg(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   token = *tl.begin();

   remove_blank(tl);

   syntax_error_if(
      tl.empty() || token.type() != tkncl_t::IDENTIFIER,
      token.expression(),
      token.position());

   std::string variable_name = token.identifier();

   --tl;
   remove_blank(tl);

   expr_any_t::handle_t variable_vector_index = nullptr;

   if (!tl.empty())
   {
      token = *tl.begin();
      token_list_t vect_etl;
      variable_vector_index = parse_sub_expr(ctx, token, tl, vect_etl);
   }

   return std::make_pair(variable_name, variable_vector_index);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_input(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   // input #<filenumber>
   if (tl.begin()->type() == tkncl_t::IDENTIFIER)
   {
      std::string id = tl.begin()->identifier();

      //if token begins with # it should be
      //an filenumber (which should be an integer)
      if (id.size() > 1 && id.c_str()[0] == '#')
      {
         //Check if it is a integer
         id = id.substr(1, id.size() - 1);

         syntax_error_if(
            !variant_t::is_integer(id),
            token.expression(),
            token.position(),
            "input# expected to be passed a valid filenumber");

         token.set_identifier(id, token_t::case_t::LOWER);
         token.set_type(tkncl_t::INTEGRAL);

         //Replace old token with new one
         --tl;
         tl.data().push_front(token);

         return parse_input_file(ctx, token, tl);
      }
   }

   return parse_input_con(ctx, token, tl);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_input_con(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   std::string prompt;

   if (token.type() == tkncl_t::STRING_LITERAL)
   {
      prompt = token.identifier();

      extract_next_token(tl, token);

      syntax_error_if(
         (token.type() != tkncl_t::OPERATOR ||
          (token.identifier() != ";" && token.identifier() != ",")),
         token.expression(),
         token.position());

      //The semicolon will print a question mark at
      //the end of the prompt string
      if (token.identifier() == ";")
         prompt += "?";

      extract_next_token(tl, token);
   }

   var_list_t var_list;

   while (!tl.empty() &&
          (token.type() != tkncl_t::OPERATOR ||
           token.identifier() != ":"))
   {

      auto var = parse_var_arg(ctx, token, tl);
      var_list.push_back(var);

      if (!tl.empty())
      {
         token = *tl.begin();

         syntax_error_if(token.type() != tkncl_t::OPERATOR ||
                         (token.identifier() != "," && token.identifier() != ":"),
                         token.expression(),
                         token.position());

         if (token.identifier() == ":" && token.type() == tkncl_t::OPERATOR)
         {
            break;
         }

         extract_next_token(
            tl,
            token);

         token = *tl.begin();
      }
   }

   syntax_error_if(var_list.empty(), token.expression(), token.position());

   return stmt_t::handle_t(
             std::make_shared<stmt_input_t>(ctx, prompt, var_list));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_input_file(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;

   int fd = 0;
   var_list_t vlist;

   parse_fd_args(ctx, token, tl, fd, vlist);

   return stmt_t::handle_t(std::make_shared<stmt_input_file_t>(ctx, fd, vlist));
}


/* -------------------------------------------------------------------------- */

void stmt_parser_t::parse_fd_args(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl,
   int & fd,
   var_list_t & var_list)
{
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(token.type() != tkncl_t::INTEGRAL,
                   token.expression(),
                   token.position());

   try
   {
      fd = nu::stoi(token.identifier());
   }
   catch (...)
   {
      syntax_error(token.expression(), token.position());
   }

   --tl;
   remove_blank(tl);

   if (!tl.empty()) // must be a comma separator
   {
      token = *tl.begin();
      syntax_error_if(
         token.type() != tkncl_t::OPERATOR ||
         token.identifier() != ",",
         token.expression(),
         token.position());
      --tl; // remove comma from tl
   }

   remove_blank(tl);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   while (!tl.empty() &&
          (token.type() != tkncl_t::OPERATOR ||
           token.identifier() != ":"))
   {

      auto var = parse_var_arg(ctx, token, tl);
      var_list.push_back(var);

      if (!tl.empty())
      {
         token = *tl.begin();

         syntax_error_if(
            token.type() != tkncl_t::OPERATOR ||
            token.identifier() != ",",
            token.expression(),
            token.position());

         extract_next_token(
            tl,
            token);

         token = *tl.begin();
      }
   }

   syntax_error_if(var_list.empty(), token.expression(), token.position());
}


/* -------------------------------------------------------------------------- */

// parse statement 'READ filenumber, variable, sizeexpr'
stmt_t::handle_t stmt_parser_t::parse_read(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;

   int fd = 0;

   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(token.type() != tkncl_t::INTEGRAL,
                   token.expression(),
                   token.position());

   try
   {
      fd = nu::stoi(token.identifier());
   }
   catch (...)
   {
      syntax_error(token.expression(), token.position());
   }

   --tl;
   remove_blank(tl);

   if (!tl.empty()) // must be a comma separator
   {
      token = *tl.begin();
      syntax_error_if(
         token.type() != tkncl_t::OPERATOR ||
         token.identifier() != ",",
         token.expression(),
         token.position());
      --tl; // remove comma from tl
   }

   remove_blank(tl);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   var_arg_t var = parse_var_arg(ctx, token, tl);

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::OPERATOR ||
      token.identifier() != ",",
      token.expression(),
      token.position());

   extract_next_token(
      tl,
      token);

   token = *tl.begin();

   syntax_error_if(tl.empty(), token.expression(), token.position());

   remove_blank(tl);

   return parse_arg_list<stmt_read_t, 1>(
      ctx,
      token,
      tl,
      [](const token_t&t)
      {
         return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
      },
      ctx,
      fd,
      var
   );
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_locate(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   return parse_arg_list<stmt_locate_t, 0 /*-> unlimited*/>(
      ctx,
      token,
      tl,
      [](const token_t&t)
      {
         return t.type() == tkncl_t::OPERATOR &&  t.identifier() == ",";
      },
      ctx
   );
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_delay(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   return parse_arg_list<stmt_delay_t, 0>(
      ctx,
      token,
      tl,
      [](const token_t&t)
      {
         return t.type() == tkncl_t::OPERATOR &&  t.identifier() == ",";
      },
      ctx
      );
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_mdelay(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   return parse_arg_list<stmt_mdelay_t, 0 /*-> unlimited*/>(
      ctx,
      token,
      tl,
      [](const token_t&t)
      {
         return t.type() == tkncl_t::OPERATOR &&  t.identifier() == ",";
      },
      ctx
   );
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_expr(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   if (token.type() == tkncl_t::STRING_LITERAL && tl.size() == 1)
   {
      --tl;

      //Create statement to print out a literal string
      return stmt_t::handle_t(
                std::make_shared<stmt_expr_t>(ctx, token.identifier()));
   }

   return 
      parse_arg_list<stmt_expr_t, 0>(
         ctx,
         token,
         tl,
         [](const token_t&t)
         {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
         },
         ctx);
}



/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_block(
   prog_ctx_t & ctx,
   nu::token_list_t & tl,
   const std::string& end_block_id,
   tkncl_t end_block_id_cl)
{
   remove_blank(tl);

   if (tl.empty())
      return stmt_empty_t::handle_t(std::make_shared<stmt_empty_t>(ctx));

   stmt_block_t::handle_t hblock(std::make_shared<stmt_block_t>(ctx));

   while (!tl.empty())
   {
      remove_blank(tl);
      (*hblock) += parse_stmt(ctx, tl);

      remove_blank(tl);

      if (tl.empty())
         break;

      token_t token(*tl.begin());

      if (!end_block_id.empty() &&
            end_block_id == token.identifier() &&
            end_block_id_cl == token.type())
      {
         --tl;
         break;
      }

      syntax_error_if(
         token.type() != tkncl_t::OPERATOR && token.identifier() != ":",
         token.expression(),
         token.position());

      --tl;
   }

   return hblock;
}


/* -------------------------------------------------------------------------- */

template<class T>
stmt_t::handle_t stmt_parser_t::parse_branch_instr(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   remove_blank(tl);

   if (! tl.empty() &&
         (tl.begin()->identifier() == "to" ||
          tl.begin()->identifier() == "sub"))
   {
      --tl;
      remove_blank(tl);

      if (!tl.empty())
         token = *tl.begin();
   }

   syntax_error_if(
      tl.empty() ||
      (token.type() != tkncl_t::INTEGRAL
       &&
       token.type() != tkncl_t::IDENTIFIER),
      token.expression(),
      token.position());

   std::string label = token.identifier();

   --tl;

   if (variant_t::is_integer(label))
   {
      prog_pointer_t::line_number_t ln = nu::stoi(label);
      syntax_error_if(ln <= 0, token.expression(), token.position());

      ctx.prog_label.set_explicit_line_reference_mode();

      return stmt_t::handle_t(std::make_shared<T>(ctx, ln));
   }

   return stmt_t::handle_t(std::make_shared<T>(ctx, label));
}


//-------------------------------------------------------------------------

expr_any_t::handle_t stmt_parser_t::parse_sub_expr(
   prog_ctx_t & ctx,
   token_t token,
   token_list_t & tl,
   token_list_t & etl)
{
   expr_any_t::handle_t sub_exp = nullptr;

   if (token.type() == tkncl_t::SUBEXP_BEGIN)
   {
      int parenthesis_level = 1;

      extract_next_token(tl, token);

      while (!tl.empty())
      {
         token = *tl.begin();
         --tl;
         remove_blank(tl);

         if (token.type() == tkncl_t::SUBEXP_BEGIN)
            ++parenthesis_level;

         else if (token.type() == tkncl_t::SUBEXP_END)
         {
            --parenthesis_level;

            if (parenthesis_level<1)
               break;
         }

         etl += token;

      }

      syntax_error_if(parenthesis_level != 0 ||
                      etl.empty(), token.expression(), token.position());
   }

   if (!etl.empty())
   {
      expr_parser_t ep;
      sub_exp = ep.compile(etl, token.position());
   }

   return sub_exp;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_let(
   prog_ctx_t & ctx,
   nu::token_list_t & tl)
{
   remove_blank(tl);

   if (tl.empty())
      return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
   
   token_t token(*tl.begin());
   size_t pos = token.position();
   std::string expr = token.expression();

   syntax_error_if(token.type() != tkncl_t::IDENTIFIER, expr, pos);

   std::string identifier = token.identifier();
   
   --tl;
   remove_blank(tl);

   if (tl.empty())
      return stmt_t::handle_t(
                std::make_shared<stmt_call_t>(identifier, ctx));

   token = *tl.begin();

   if (token.type() != tkncl_t::OPERATOR &&
         token.type() != tkncl_t::SUBEXP_BEGIN)
   {
      return parse_arg_list<stmt_call_t, 0>(
         ctx,
         token,
         tl,
         [](const token_t&t)
         {
            return t.type() == tkncl_t::OPERATOR && t.identifier() == ",";
         },
         identifier,
         ctx
      );
   }

   token_list_t vect_etl;
   expr_any_t::handle_t variable_vector_index  = 
      parse_sub_expr( ctx, token, tl, vect_etl );

   remove_blank(tl);
   syntax_error_if(tl.empty(), expr, pos);
   token = *tl.begin();

   bool struct_member_id = false;

   nu::expr_any_t::handle_t struct_member_vector_index;

   if (token.type() == tkncl_t::OPERATOR && token.identifier() == ".")
   {
      --tl;
      remove_blank(tl);
      syntax_error_if(tl.empty(), expr, pos);

      token = *tl.begin();

      syntax_error_if(token.type() != tkncl_t::IDENTIFIER, expr, pos);

      identifier += ".";
      identifier += token.identifier();

      --tl;
      remove_blank(tl);
      syntax_error_if(tl.empty(), expr, pos);

      token = *tl.begin();

      token_list_t etl;

      if (token.type() == tkncl_t::SUBEXP_BEGIN)
      {
         move_sub_expression(
            tl,                     // source
            etl,                    // destination
            "=", tkncl_t::OPERATOR  // end-of-expression
            );

         expr_parser_t ep;
         struct_member_vector_index = ep.compile(etl, pos);

         syntax_error_if(
            tl.empty(),
            token.expression(),
            token.position());

         token = *tl.begin();
      }

      struct_member_id = true;
   }
  
   syntax_error_if(token.type() != tkncl_t::OPERATOR ||
                   token.identifier() != "=", expr, pos);

   --tl;
   remove_blank(tl);

   expr_parser_t ep;
   pos = token.position();

   token_list_t etl;
   remove_blank(tl);

   move_sub_expression(
      tl,                     // source
      etl,                    // destination
      ":", tkncl_t::OPERATOR  // end-of-expression
   );

   syntax_error_if(
      !variable_t::is_valid_name(identifier, false),
      identifier + " is an invalid identifier");

   return stmt_t::handle_t(
             std::make_shared<stmt_let_t>(
                ctx,
                identifier,
                ep.compile(etl, pos),
                variable_vector_index,
                struct_member_vector_index,
                struct_member_id));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_for_to_step(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   size_t pos = token.position();
   std::string expr = token.expression();

   syntax_error_if(
      tl.empty() || token.type() != tkncl_t::IDENTIFIER,
      expr,
      pos);

   token = *tl.begin();
   --tl;
   remove_blank(tl);

   syntax_error_if(tl.empty(), expr, pos);

   std::string variable_name = token.identifier();

   syntax_error_if(!
      variable_t::is_valid_name(variable_name, false),
      variable_name + " is an invalid identifier");

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::OPERATOR || token.identifier() != "=",
      token.expression(),
      token.position());

   --tl;
   remove_blank(tl);

   expr_parser_t ep;
   pos = token.position();

   token_list_t etl;
   remove_blank(tl);

   move_sub_expression(
      tl,                       // source
      etl,                      // destination
      "to", tkncl_t::IDENTIFIER // end-of-expression
   );

   extract_next_token(tl, token);

   auto from_expr = ep.compile(etl, pos);
   etl.clear();

   while (!tl.empty() &&
          (tl.begin()->identifier() != "step"&&
           tl.begin()->identifier() != ":"))
   {
      const token_t token(*tl.begin());
      etl += token;
      --tl;
      remove_blank(tl);
   }

   expr_any_t::handle_t to_expr = ep.compile(etl, pos);
   etl.clear();
   expr_any_t::handle_t step(std::make_shared<expr_literal_t>(1));


   if (!tl.empty() && tl.begin()->identifier() == "step")
   {
      pos = tl.begin()->position();
      --tl;
      remove_blank(tl);

      move_sub_expression(
         tl,                       // source
         etl,                      // destination
         ":", tkncl_t::OPERATOR    // end-of-expression
      );

      step = ep.compile(etl, pos);
   }

   return stmt_t::handle_t(
             std::make_shared<stmt_for_to_step_t>(
                ctx, variable_name, from_expr, to_expr, step));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_next(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   std::string variable_name;

   if (!tl.empty())
   {
      token = *tl.begin();

      if (! (token.identifier() == ":" && token.type() == tkncl_t::OPERATOR))
      {
         syntax_error_if(
            token.type() != tkncl_t::IDENTIFIER,
            token.expression(),
            token.position());

         variable_name = token.identifier();

         syntax_error_if(!
            variable_t::is_valid_name(variable_name, false),
            variable_name + " is an invalid identifier");

         --tl;
         remove_blank(tl);
      }

   }

   return stmt_t::handle_t(std::make_shared<stmt_next_t>(ctx, variable_name));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_exit(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER ||
      (token.identifier() != "for" &&
       token.identifier() != "sub" &&
       token.identifier() != "function" &&
       token.identifier() != "while" &&
       token.identifier() != "do"),
      token.expression(),
      token.position());

   --tl;
   remove_blank(tl);

   if (token.identifier() == "while")
      return stmt_t::handle_t(std::make_shared<stmt_exit_while_t>(ctx));

   else if (token.identifier() == "do")
      return stmt_t::handle_t(std::make_shared<stmt_exit_do_t>(ctx));

   else if (token.identifier() == "sub" || token.identifier() == "function")
      return stmt_t::handle_t(std::make_shared<stmt_exit_sub_t>(ctx));

   else if (token.identifier() == "for")
      return stmt_t::handle_t(std::make_shared<stmt_exit_for_t>(ctx));

   //throws an exception
   syntax_error_if(
      true,
      token.expression(),
      token.position());

   //never executed
   return nullptr;
}


/* -------------------------------------------------------------------------- */

template<class T>
stmt_t::handle_t stmt_parser_t::parse_procedure(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   //Skip keyword SUB
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER,
      token.expression(),
      token.position());

   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   std::string id = token.identifier();

   syntax_error_if(
      !variable_t::is_valid_name(id, false), 
      id + " is an invalid identifier");

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::SUBEXP_BEGIN,
      token.expression(),
      token.position());

   --tl;
   remove_blank(tl);

   token = *tl.begin();

   stmt_t::handle_t stmt_handle;

   if (token.type() == tkncl_t::SUBEXP_END)
   {
      --tl;
      remove_blank(tl);
            
      stmt_handle = stmt_t::handle_t(std::make_shared<T>(ctx, id));
   }
   else
   {
      //reset '(' before other tokens
      tl.data().push_front(token);
      stmt_handle = parse_parameter_list<T>(ctx, token, tl, ")", ctx, id);
   }
      

   if (stmt_handle &&
      !tl.empty() && 
      tl.begin()->type()==tkncl_t::IDENTIFIER && 
      tl.begin()->identifier()=="as")
   {
      --tl;
      remove_blank(tl);

      syntax_error_if(
         tl.empty() || tl.begin()->type() != tkncl_t::IDENTIFIER,
         token.expression(),
         token.position());

      auto ret_type = tl.begin()->identifier();

      stmt_function_t * ptr = 
         dynamic_cast<stmt_function_t*>(stmt_handle.get());

      syntax_error_if(
         ptr == nullptr,
         token.expression(),
         token.position());

      --tl;
      remove_blank(tl);

      ptr->define_ret_type(ret_type, ctx);
   }


   return stmt_handle;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_struct(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   //Skip keyword STRUCT
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER,
      token.expression(),
      token.position());

   const std::string& id = token.identifier();

   syntax_error_if(
      !variable_t::is_valid_name(id, false), 
      token.expression(),
      token.position(),
      "'" + id + "' is an invalid identifier");

   --tl;
   remove_blank(tl);
   syntax_error_if(!tl.empty(), token.expression(), token.position());

   ctx.compiling_struct_name = id;

   return stmt_t::handle_t(std::make_shared<stmt_struct_t>(ctx, id));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_struct_element(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER,
      token.expression(),
      token.position());

   std::string id = token.identifier();

   syntax_error_if(
      !variable_t::is_valid_name(id, false),
      token.expression(),
      token.position(),
      "'" + id + "' is an invalid identifier");

   --tl;
   remove_blank(tl);

   auto type = 
      variable_t::typename_by_type(variable_t::type_by_name(id));

   size_t size = 0;
   
   if (tl.empty())
      return stmt_t::handle_t(
         std::make_shared<stmt_struct_element_t>(ctx, id, type, size));

   token = *tl.begin();

   syntax_error_if(
      ! ((token.type() == tkncl_t::IDENTIFIER && token.identifier() == "as") ||
           token.type() == tkncl_t::SUBEXP_BEGIN),
      token.expression(),
      token.position());
      
   if (token.type() == tkncl_t::SUBEXP_BEGIN)
   {
      --tl;
      remove_blank(tl);
      syntax_error_if(tl.empty(), token.expression(), token.position());

      token = *tl.begin();

      syntax_error_if(
         token.type() != tkncl_t::INTEGRAL,
         token.expression(),
         token.position());

      try
      {
         size = size_t(nu::stoll(token.identifier()));
      }
      catch (...)
      {
         syntax_error(token.expression(), token.position());
      }

      --tl;
      remove_blank(tl);
      syntax_error_if(tl.empty(), token.expression(), token.position());

      token = *tl.begin();

      syntax_error_if(
         token.type() != tkncl_t::SUBEXP_END,
         token.expression(),
         token.position(),
         "'" + id + "' is an invalid identifier");

      --tl;
      remove_blank(tl);
   }
      
   if (tl.empty())
      return stmt_t::handle_t(
         std::make_shared<stmt_struct_element_t>(ctx, id, type, size));

   token = *tl.begin();

   if (token.type() == tkncl_t::IDENTIFIER && token.identifier() == "as")
   {
      --tl;
      remove_blank(tl);

      syntax_error_if(
         tl.empty(),
         token.expression(),
         token.position());

      token = *tl.begin();

      syntax_error_if(
         token.type() != tkncl_t::IDENTIFIER,
         token.expression(),
         token.position());

      type = token.identifier();

      --tl;
      remove_blank(tl);
   }

   return stmt_t::handle_t(std::make_shared<stmt_struct_element_t>(ctx, id, type, size));
}

/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_if_then_else(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   expr_parser_t ep;
   token_list_t etl;

   // Move tokens from tl to etl until "then" is found
   move_sub_expression(
      tl,     // source token list
      etl,    // expression token list
      "then", // end-of-list-identifier
      tkncl_t::IDENTIFIER);

   remove_blank(tl);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

   //skip "then"
   --tl;
   remove_blank(tl);

   etl.clear();

   // Copy tokens from tl to etl until "else" is found
   move_sub_expression(
      tl,      // source token list
      etl,     // destination token list
      "else",  // end-of-list-identifier
      tkncl_t::IDENTIFIER);

   if (!tl.empty())
   {
      //skip "else"
      --tl;
      remove_blank(tl);

      syntax_error_if(tl.empty(),"ELSE: expected a statement");
   }

   stmt_t::handle_t then_stmt = parse_block(ctx, etl);

   remove_blank(tl);

   stmt_t::handle_t else_stmt = parse_block(ctx, tl);

   return stmt_t::handle_t(
             std::make_shared<stmt_if_then_else_t>(ctx, condition, then_stmt, else_stmt));
}


//! parse elif-statement
stmt_t::handle_t stmt_parser_t::parse_elif_stmt(
   prog_ctx_t & ctx,
   token_t token,
   token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.rbegin();
   tl--;
   remove_blank(tl);

   syntax_error_if(
      token.identifier() != "then" ||
      token.type() != tkncl_t::IDENTIFIER ||
      tl.empty(), token.expression(), token.position());

   expr_parser_t ep;
   expr_any_t::handle_t condition = ep.compile(tl, token.position() + 1);
   tl.clear();

   return stmt_t::handle_t(std::make_shared<stmt_elif_t>(ctx, condition));
}


/* -------------------------------------------------------------------------- */

//! parse instruction parameter list
stmt_on_goto_t::label_list_t stmt_parser_t::parse_label_list(
   prog_ctx_t & ctx,
   nu::token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   stmt_on_goto_t::label_list_t lbl_list;

   while (!tl.empty() &&
          (tl.begin()->type() != tkncl_t::OPERATOR &&
           tl.begin()->identifier() != ":"))
   {
      token = *tl.begin();

      syntax_error_if(token.type() != tkncl_t::IDENTIFIER,
                      token.expression(), token.position());

      std::string label_name = token.identifier();

      lbl_list.push_back(label_name);

      --tl;
      remove_blank(tl);

      if (tl.empty())
         break;

      token = *tl.begin();

      syntax_error_if(
         (token.type() != tkncl_t::OPERATOR &&
          token.type() != tkncl_t::SUBEXP_END)
         ||
         (token.identifier() != "," && token.identifier() != ":"),
         token.expression(), token.position());

      if (token.identifier() == ":")
         break;

      --tl;
      remove_blank(tl);
   }

   return lbl_list;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_on_goto(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   expr_parser_t ep;
   token_list_t etl;

   // Move tokens from tl to etl until "then" is found
   move_sub_expression(
      tl,     // source token list
      etl,    // expression token list
      "goto", // end-of-list-identifier
      tkncl_t::IDENTIFIER);

   remove_blank(tl);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

   //args
   auto label_list = parse_label_list(ctx, token, tl);

   return stmt_t::handle_t(
      std::make_shared<stmt_on_goto_t>(ctx, condition, label_list));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_while(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   expr_parser_t ep;
   token_list_t etl;

   // Move tokens from tl to etl until "then" is found
   move_sub_expression(
      tl,   // source token list
      etl,  // expression token list
      "do", // end-of-list-identifier
      tkncl_t::IDENTIFIER);

   remove_blank(tl);

   expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

   if (!tl.empty())
   {
      //skip "do"
      --tl;
      remove_blank(tl);

      if (!tl.empty())
      {
         stmt_t::handle_t while_stmt = parse_block(ctx, tl);

         //while <condition> do <stmt>
         return stmt_t::handle_t(
            std::make_shared<stmt_while_t>(ctx, condition, while_stmt));
      }
   }

   //while <condition> [do]
   return stmt_t::handle_t(
      std::make_shared<stmt_while_t>(ctx, condition));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_loop_while(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());


   token = *tl.begin();
   --tl;
   remove_blank(tl);

   syntax_error_if(
      token.identifier() != "while" ||
      token.type() != tkncl_t::IDENTIFIER ||
      tl.empty(), token.expression(), token.position());


   expr_parser_t ep;
   token_list_t etl;

   // Move tokens from tl to etl until "then" is found
   move_sub_expression(
      tl,   // source token list
      etl,  // expression token list
      ":", // end-of-list-identifier
      tkncl_t::OPERATOR);

   remove_blank(tl);

   expr_any_t::handle_t condition = ep.compile(etl, token.position() + 1);

   //LOOP WHILE <condition>
   return stmt_t::handle_t(
      std::make_shared<stmt_loop_while_t>(ctx, condition));
}


/* -------------------------------------------------------------------------- */

// Open pathname For mode [Access access] As [#]filenumber
stmt_t::handle_t stmt_parser_t::parse_open(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   expr_parser_t ep;
   token_list_t etl;

   // Move tokens from tl to etl until "then" is found
   move_sub_expression(
      tl,     // source token list
      etl,    // expression token list
      "for", // end-of-list-identifier
      tkncl_t::IDENTIFIER);

   remove_blank(tl);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   expr_any_t::handle_t filename = ep.compile(etl, token.position() + 1);

   //skip "for"
   extract_next_token(tl, token);

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER,
      token.expression(),
      token.position());

   std::string mode = token.identifier();

   syntax_error_if(
      mode != "input" && mode != "output" &&
      mode != "random" && mode != "append",
      token.expression(),
      token.position());

   //skip mode
   extract_next_token(tl, token);
   std::string access = "";

   if (token.type() == tkncl_t::IDENTIFIER && token.identifier() == "access")
   {
      extract_next_token(tl, token);

      access = token.identifier();

      syntax_error_if(
         token.type() != tkncl_t::IDENTIFIER,
         token.expression(),
         token.position());

      syntax_error_if(
         access != "read" && access != "write",
         token.expression(),
         token.position());

      extract_next_token(tl, token);
   }


   if (access == "read")
   {
      syntax_error_if(
         token.type() != tkncl_t::IDENTIFIER ||
         (token.identifier() != "write" && token.identifier() != "as"),
         token.expression(),
         token.position());

      if (token.identifier() == "write")
      {
         access += " write";
         extract_next_token(tl, token);
      }
   }

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER ||
      token.identifier() != "as",
      token.expression(),
      token.position());

   //skip mode
   extract_next_token(tl, token);

   --tl; //remove #<filenumber> token from tl

   std::string fds = token.identifier();

   if (fds.size() > 1 && fds.c_str()[0] == '#')
      fds = fds.substr(1, fds.size() - 1);

   int fd = 0;

   try
   {
      fd = nu::stoi(fds);
   }
   catch (...)
   {
      syntax_error(
         token.expression(),
         token.position());
   }

   return stmt_t::handle_t(
      std::make_shared<stmt_open_t>(ctx, filename, mode, access, fd));
}


/* -------------------------------------------------------------------------- */

// close [#]filenumber
stmt_t::handle_t stmt_parser_t::parse_close(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   //skip mode
   --tl;
   extract_next_token(tl, token);

   --tl; //remove #<filenumber> token from tl

   std::string fds = token.identifier();

   if (fds.size() > 1 && fds.c_str()[0] == '#')
      fds = fds.substr(1, fds.size() - 1);

   int fd = 0;

   try
   {
      fd = nu::stoi(fds);
   }
   catch (...)
   {
      syntax_error(
         token.expression(),
         token.position());
   }

   return stmt_t::handle_t(std::make_shared<stmt_close_t>(ctx, fd));
}


//! parse instruction const
stmt_t::handle_t stmt_parser_t::parse_const(
   prog_ctx_t & ctx,
   token_t token,
   token_list_t & tl)
{
   
   auto get_type = [&](
      std::string& type,
      const std::string& variable_name)
   {
      type =
         variable_t::typename_by_type(
            variable_t::type_by_name(variable_name));

      if (!tl.empty())
      {
         token = *tl.begin();

         if (token.type() == tkncl_t::IDENTIFIER &&
            token.identifier() == "as")
         {
            --tl;
            remove_blank(tl);

            syntax_error_if(
               tl.empty(),
               token.expression(),
               token.position());

            token = *tl.begin();

            syntax_error_if(
               token.type() != tkncl_t::IDENTIFIER,
               token.expression(),
               token.position());

            type = token.identifier();

            --tl;
            remove_blank(tl);
         }
      }
   };

   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER,
      token.expression(), 
      token.position());

   std::string variable_name = token.identifier();

   syntax_error_if(!
      variable_t::is_valid_name(variable_name, false),
      variable_name + " is an invalid identifier");

   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());
   token = *tl.begin();
      
   std::string type;
   get_type(type, variable_name);

   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   syntax_error_if(
      token.type() != tkncl_t::OPERATOR ||
      token.identifier() != "=",
      token.expression(), token.position());

   --tl;
   remove_blank(tl);

   expr_parser_t ep;
   auto pos = token.position();

   token_list_t etl;
   remove_blank(tl);

   move_sub_expression(
      tl,                     // source
      etl,                    // destination
      ":", tkncl_t::OPERATOR  // end-of-expression
      );

   syntax_error_if(
      !variable_t::is_valid_name(variable_name, false),
      variable_name + " is an invalid identifier");

   return std::make_shared<stmt_const_t>(ctx, variable_name, type, ep.compile(etl, pos));
}


/* -------------------------------------------------------------------------- */

// parse 'label_identifier:'
stmt_t::handle_t stmt_parser_t::parse_label(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   //Parse label
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   //Check for new label
   if (tl.size() > 1 &&
         (tl.begin() + 1)->type() == tkncl_t::OPERATOR &&
         (tl.begin() + 1)->identifier() == ":")
   {
      label_tbl_t& prog_label = ctx.prog_label;

      const std::string& label = token.identifier();

      if (prog_label.is_defined(label))
      {
         syntax_error_if(int(prog_label[label]) != _parsing_line,
                         "Label '" + token.identifier() +
                         "' alrady defined at " + nu::to_string(_parsing_line));
      }
      else
      {
         prog_label.define(token.identifier(), _parsing_line);
      }

      --tl; // remove tokens '<LABEL>' ':'
      remove_blank(tl);
      --tl;
      remove_blank(tl);

      return tl.empty() ? 
         std::make_shared<stmt_empty_t>(ctx) :
         parse_block(ctx, tl);
   }

   return nullptr;
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_goto_gosub(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   std::string instruction = token.identifier();
   bool go_blank_to = instruction == "go";

   --tl;
   remove_blank(tl);
   syntax_error_if(tl.empty(), token.expression(), token.position());

   token = *tl.begin();

   std::string identifier = token.identifier();

   syntax_error_if(go_blank_to && identifier != "to" && identifier != "sub",
                   token.expression(), token.position());

   return ((instruction.find("to") != std::string::npos) ?
           parse_branch_instr<stmt_goto_t>(ctx, token, tl) :
           parse_branch_instr<stmt_gosub_t>(ctx, token, tl));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_end(
   prog_ctx_t & ctx,
   token_t token,
   nu::token_list_t & tl)
{
   --tl;
   remove_blank(tl);
      
   if (!tl.empty())
   {
      token_t token = *tl.begin();

      syntax_error_if(
         token.type() != tkncl_t::IDENTIFIER,
         token.expression(),
         token.position());

      const auto& id = token.identifier();

      if (id == "while")
      {
         --tl;
         return stmt_t::handle_t(std::make_shared<stmt_wend_t>(ctx));
      }
      else if (id == "if")
      {
         --tl;
         return stmt_t::handle_t(std::make_shared<stmt_endif_t>(ctx));
      }
      else if (id == "sub")
      {
         --tl;
         return stmt_t::handle_t(std::make_shared<stmt_endsub_t>(ctx));
      }
      else if (id == "function")
      {
         --tl;
         return stmt_t::handle_t(std::make_shared<stmt_endfunction_t>(ctx));
      }
      else if (id == "struct")
      {
         ctx.compiling_struct_name.clear();;

         --tl;
         return stmt_t::handle_t(std::make_shared<stmt_endstruct_t>(ctx));
      }
      else
      {
         syntax_error(token.expression(), token.position());
      }
   }

   return stmt_t::handle_t(std::make_shared<stmt_end_t>(ctx));
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::parse_stmt(
   prog_ctx_t & ctx,
   nu::token_list_t & tl)
{
   remove_blank(tl);

   if (tl.empty())
      return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));

   token_t token(*tl.begin());

   if (token.type() == tkncl_t::NEWLINE)
      return std::make_shared<stmt_empty_t>(ctx);

   const std::string& identifier = token.identifier();

   if (!ctx.compiling_struct_name.empty())
   {
      if (! 
          ((token.identifier() == "end" ||
            token.identifier() == "rem" ) 
           &&
            token.type() == tkncl_t::IDENTIFIER))
      {
         return parse_struct_element(ctx, token, tl);
      }
   }

   syntax_error_if(
      (token.type() != tkncl_t::IDENTIFIER) &&
      (token.type() == tkncl_t::OPERATOR &&
       (token.identifier() != NU_BASIC_OP_INC &&
        (token.identifier() != NU_BASIC_OP_DEC))),
      token.expression(),
      token.position());

   if (identifier == NU_BASIC_OP_INC || identifier == NU_BASIC_OP_DEC)
      return parse_expr(ctx, token, tl);

#ifndef TINY_NUBASIC_VER

   if (identifier == "textout")
      return parse_generic_instruction<stmt_textout_t>(ctx, token, tl, ctx);

   if (identifier == "plotimage")
      return parse_generic_instruction<stmt_plotimage_t>(ctx, token, tl, ctx);

   if (identifier == "line")
      return parse_generic_instruction<stmt_line_t>(ctx, token, tl, ctx);

   if (identifier == "setpixel")
      return parse_generic_instruction<stmt_set_pixel_t>(ctx, token, tl, ctx);

   if (identifier == "rect")
      return parse_generic_instruction<stmt_rect_t>(ctx, token, tl, ctx);

   if (identifier == "fillrect")
      return parse_generic_instruction<stmt_fillrect_t>(ctx, token, tl, ctx);

   if (identifier == "ellipse")
      return parse_generic_instruction<stmt_ellipse_t>(ctx, token, tl, ctx);

   if (identifier == "fillellipse")
      return parse_generic_instruction<stmt_fillellipse_t>(ctx, token, tl, ctx);

#endif // TINY_NUBASIC_VER

   if (identifier == "print"  ||
         identifier == "write"  ||
         identifier == "print#" ||
         identifier == "write#" )
   {
      return parse_print(ctx, token, tl);
   }

   if (identifier == "locate")
      return parse_locate(ctx, token, tl);

   if (identifier == "delay" || identifier == "sleep")
      return parse_delay(ctx, token, tl);

   if (identifier == "mdelay")
      return parse_mdelay(ctx, token, tl);

   if (identifier == "input")
      return parse_input(ctx, token, tl);

   if (identifier == "input#")
      return parse_input_file(ctx, token, tl);

   if (identifier == "read")
      return parse_read(ctx, token, tl);

   if (identifier == "on")
      return parse_on_goto(ctx, token, tl);

   if (identifier == "go" || identifier == "goto" || identifier == "gosub")
      return parse_goto_gosub(ctx, token, tl);

   if (identifier == "rem")
   {
      tl.clear();
      return stmt_t::handle_t(std::make_shared<stmt_empty_t>(ctx));
   }

   if (identifier == "end")
      return parse_end(ctx, token, tl);

   if (identifier == "wend")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_wend_t>(ctx));
   }

   if (identifier == "do")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_do_t>(ctx));
   }

   if (identifier == "else")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_else_t>(ctx));
   }

   if (identifier == "return")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_return_t>(ctx));
   }

   if (identifier == "if")
      return parse_if_then_else(ctx, token, tl);

   if (identifier == "elif" || identifier == "elseif")
      return parse_elif_stmt(ctx, token, tl);

   if (identifier == "for")
      return parse_for_to_step(ctx, token, tl);

   if (identifier == "next")
      return parse_next(ctx, token, tl);

   if (identifier == "exit")
      return parse_exit(ctx, token, tl);

   if (identifier == "beep")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_beep_t>(ctx));
   }

   if (identifier == "cls")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_cls_t>(ctx));
   }

   if (identifier == "randomize")
   {
      --tl;
      return stmt_t::handle_t(std::make_shared<stmt_randomize_t>(ctx));
   }

   if (identifier == "const")
      return parse_const(ctx, token, tl);

   if (identifier == "dim")
     return parse_parameter_list<stmt_dim_t>(ctx, token, tl, ":", ctx);

   if (identifier == "redim")
      return parse_parameter_list<stmt_redim_t>(ctx, token, tl, ":", ctx);

   if (identifier == "open")
      return parse_open(ctx, token, tl);

   if (identifier == "close")
      return parse_close(ctx, token, tl);

   if (identifier == "while")
      return parse_while(ctx, token, tl);

   if (identifier == "loop")
      return parse_loop_while(ctx, token, tl);

   if (identifier == "shell")
      return parse_generic_instruction<stmt_shell_t>(ctx, token, tl, ctx);

   if (identifier == "chdir")
      return parse_generic_instruction<stmt_chdir_t>(ctx, token, tl, ctx);

   if (identifier == "fopen")
      return parse_generic_instruction<stmt_fopen_t>(ctx, token, tl, ctx);

   if (identifier == "seek")
      return parse_generic_instruction<stmt_fseek_t>(ctx, token, tl, ctx);

   if (identifier == "flush")
      return parse_generic_instruction<stmt_flush_t>(ctx, token, tl, ctx);

   if (identifier == "sub")
      return parse_procedure<stmt_sub_t>(ctx, token, tl);

   if (identifier == "struct")
      return parse_struct(ctx, token, tl);

   if (identifier == "function")
      return parse_procedure<stmt_function_t>(ctx, token, tl);
   else
   {
      if (identifier == "let")
      {
         --tl;
      }
      else
      {
         auto h = parse_label(ctx, token, tl);

         if (h)
            return h;
      }

      return parse_let(ctx, tl);
   }

   throw exception_t("Syntax error");
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::compile_line(
   nu::token_list_t & tl,
   prog_ctx_t & ctx)
{
   _parsing_line = ctx.compiletime_pc.get_line();
   _parsing_stmt = ctx.compiletime_pc.get_stmt_pos();

   if (ctx.tracing_on)
   {
      std::stringstream log;

      for (auto t : tl)
      {
         log << token_t::description_of(t.type())
             << "[" << t.identifier() << "] ";
      }

      fprintf(ctx.get_stdout_ptr(), "%s\n", log.str().c_str());
   }

   remove_blank(tl);

   if (!tl.empty())
   {
      token_t token(*tl.begin());

      //syntax check
      switch (token.type())
      {
         case tkncl_t::IDENTIFIER:
            break;

         case tkncl_t::OPERATOR:
            syntax_error_if(
               token.identifier() != NU_BASIC_OP_INC &&
               token.identifier() != NU_BASIC_OP_INC,
               token.expression(),
               token.position());
            break;

         default:
            syntax_error(token.expression(), token.position());
      }

      return parse_block(ctx, tl);
   }

   return std::make_shared<stmt_empty_t>(ctx);
}


/* -------------------------------------------------------------------------- */

stmt_t::handle_t stmt_parser_t::compile_line(
   nu::expr_tknzr_t & st,
   prog_ctx_t & ctx)
{
   nu::token_list_t tl;
   st.get_tknlst(tl);
   return compile_line(tl, ctx);
}


/* -------------------------------------------------------------------------- */

bool stmt_parser_t::search_for__as_type(
   const nu::token_list_t & tl, 
   std::string & tname)
{
   nu::token_list_t etl(tl);
   remove_blank(etl);

   if ( etl.empty() )
      return false;

   auto token = *etl.begin();

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER,
      token.expression(),
      token.position());

   remove_blank(etl);

   if ( etl.empty() )
      return false;

   --etl;

   token = *etl.begin();

   if ( token.type() == tkncl_t::SUBEXP_BEGIN )
   {
      while ( !etl.empty() )
      {
         if ( token.type() == tkncl_t::SUBEXP_END )
            break;

         --etl;
      }

      if ( etl.empty() )
         return false;

      token = *etl.begin();
   }

   syntax_error_if(
      token.type() != tkncl_t::IDENTIFIER || token.identifier() != "as",
      token.expression(),
      token.position());

   return false;
}


/* -------------------------------------------------------------------------- */

} // namespace nu



