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

#include "nu_expr_tknzr.h"
#include "nu_lxa.h"
#include "nu_variant.h"

#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

expr_tknzr_t::expr_tknzr_t(
   const std::string& data,
   size_t pos,
   const std::string& blanks,
   const std::string& newlines,
   const std::string& operators,
   const std::set<std::string>& str_op,
   const char subexp_bsymb,   //call/subexpr operators
   const char subexp_esymb,
   const std::string& string_bsymb,
   const std::string& string_esymb,
   const char string_escape) :
   base_tknzr_t(data),
   _pos(pos),
   _str_op(str_op),
   _strtk(string_bsymb, string_esymb, string_escape)
{
   _subexp_begin_symb.push_back(subexp_bsymb);
   _subexp_end_symb.push_back(subexp_esymb);

   for (auto e : blanks)
      _blank.register_pattern(e);

   for (auto e : newlines)
      _newl.register_pattern(e);

   for (auto e : operators)
      _op.register_pattern(e);

   for (auto e : str_op)
      _word_op.register_pattern(e);

   _op.register_pattern(subexp_bsymb);
   _op.register_pattern(subexp_esymb);

   _comment_line_set.insert(std::make_pair("'", tkncl_t::OPERATOR));
}


/* -------------------------------------------------------------------------- */

void expr_tknzr_t::get_tknlst(token_list_t & tl, bool strip_comment)
{
   tl.clear();

   if (eol())
      return;

   do
   {
      tl.data().push_back(next());
   }
   while (!eol());

   if (strip_comment)
   {
      strip_comment_line(tl, _comment_line_set);
   }
}


/* -------------------------------------------------------------------------- */

void expr_tknzr_t::strip_comment_line(
   token_list_t& tl,
   const typed_token_set_t& comment_id_set)
{
   bool comment_found = false;
   token_list_t ntl;

   for (auto i = tl.data().begin(); i != tl.data().end(); ++i)
   {
      for (const auto & id : comment_id_set)
      {
         if (i->type() == id.second && i->identifier() == id.first)
         {
            comment_found = true;
            break;
         }
      }

      if (comment_found)
         break;

      ntl += *i;
   }

   if (comment_found)
      tl = ntl;
}


/* -------------------------------------------------------------------------- */

static inline bool identifier_char(char c)
{
   if ((c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         c == '_')
   {
      return true;
   }

   return false;
}


/* -------------------------------------------------------------------------- */

token_t expr_tknzr_t::next()
{
   // Fix . operator for real numbers

   auto pointer = tell();
   auto tkn = _next();

   if (!(tkn.type() == tkncl_t::INTEGRAL ||
      (tkn.identifier() == "." && tkn.type() == tkncl_t::OPERATOR)))
   {
      return tkn;
   }

   auto position = tkn.position();
   std::string id = tkn.identifier();

   if (tkn.type() == tkncl_t::INTEGRAL)
   {
      tkn = _next();

      if (tkn.type() != tkncl_t::OPERATOR || tkn.identifier() != ".")
      {
         set_cptr(pointer);
         return _next();
      }

      id += tkn.identifier();

      tkn = _next();

      if (tkn.type() != tkncl_t::INTEGRAL)
      {
         set_cptr(pointer);
         return _next();
      }

      id += tkn.identifier();
   }
   else if (tkn.identifier() == "." && tkn.type() == tkncl_t::OPERATOR)
   {
      tkn = _next();

      if (tkn.type() != tkncl_t::INTEGRAL)
      {
         set_cptr(pointer);
         return _next();
      }

      id = "0." + tkn.identifier();
   }
   else
   {
      set_cptr(pointer);
      return _next();
   }

   tkn.set_identifier(
      id,
      token_t::case_t::NOCHANGE);

   tkn.set_position(position);
   tkn.set_type(tkncl_t::REAL);

   return tkn;
}



/* -------------------------------------------------------------------------- */

token_t expr_tknzr_t::_next()
{
   std::string other;

   auto token_class = [&](const std::string& tk) -> tkncl_t
   {
      if (variant_t::is_integer(tk))
         return tkncl_t::INTEGRAL;
      else if (variant_t::is_real(tk))
         return tkncl_t::REAL;

      //Resolve operator like "mod", "div", ...
      std::string tklowercase = tk;

      std::transform(
         tklowercase.begin(),
         tklowercase.end(),
         tklowercase.begin(),
         tolower);

      return _str_op.find(tklowercase) != _str_op.end() ?
         tkncl_t::OPERATOR :
         tkncl_t::IDENTIFIER;
   };

   _strtk.reset();
   _word_op.reset();

   char last_symbol = 0;

   while (!eol())
   {
      token_t token = make_token(tell() + get_exp_pos(), data());
      const char symbol = get_symbol();

      _blank.reset();
      _newl.reset();
      _op.reset();

      // Detect strings...
      if (_strtk.accept(symbol) && other.empty())
      {
         seek_next();

         if (eol())
         {
            token.set_identifier(
               _strtk.data(),
               token_t::case_t::NOCHANGE);

            token.set_type(_strtk.string_complete() ?
                           tkncl_t::STRING_LITERAL :
                           tkncl_t::UNDEFINED);
            return token;
         }

         continue;
      }

      //Detect "word" operators <=, >=, <>, ...
      //They must be analyzed before "1-character" operators
      //like <,>,=, ...
      else if (_word_op.accept(symbol) &&
               (!identifier_char(last_symbol) && /*->NOTE1*/
                !identifier_char(symbol)))  /*->NOTE1*/
      {
         /*NOTE1*/
         //operators like or, and, etc... may be part of
         //an identifier such as 'for', 'mandatory', etc...
         //we have to recognize this case in order to
         //avoid to split this identifier in different
         //tokens, like f-'or' or m-'and'-t'or'y !

         auto set_point = tell();
         seek_next();
         char symbol = get_symbol();

         while (!eol() && _word_op.accept(symbol))
         {

            if (_word_op.keyword_matches)
            {
               if (other.empty())
               {
                  seek_next();

                  if (identifier_char(symbol) &&
                        identifier_char(get_symbol()))
                  {
                     /*->NOTE1*/
                     break;
                  }

                  token.set_identifier(
                     _word_op.data(), token_t::case_t::LOWER);
                  token.set_type(tkncl_t::OPERATOR);
                  return token;
               }
               else
               {
                  _word_op.reset();
                  set_cptr(set_point);
                  token.set_identifier(other, token_t::case_t::LOWER);
                  token.set_type(token_class(other));
                  return token;
               }
            }

            seek_next();
            symbol = get_symbol();
         }

         _word_op.reset();
         set_cptr(set_point);
      }

      if (_strtk.string_complete())
      {
         token.set_identifier(_strtk.data(), token_t::case_t::NOCHANGE);
         token.set_type(tkncl_t::STRING_LITERAL);
         return token;
      }


      if (_blank.accept(symbol) ||
            _newl.accept(symbol) ||
            _op.accept(symbol))
      {
         if (other.empty())
         {
            std::string s_symbol;
            s_symbol.push_back(symbol);
            token.set_identifier(s_symbol, token_t::case_t::LOWER);

            if (!_blank.data().empty())
            {
               token.set_type(tkncl_t::BLANK);
            }
            else if (!_newl.data().empty())
            {
               token.set_type(tkncl_t::NEWLINE);
            }
            else
            {
               if (token.identifier() == subexp_begin_symbol())
                  token.set_type(tkncl_t::SUBEXP_BEGIN);
               else if (token.identifier() == subexp_end_symbol())
                  token.set_type(tkncl_t::SUBEXP_END);
               else
                  token.set_type(tkncl_t::OPERATOR);
            }

            seek_next();
         }
         else
         {
            token.set_identifier(other, token_t::case_t::LOWER);
            token.set_type(token_class(other));

            other.clear();
         }

         return token;

      }

      other += symbol;
      seek_next();

      if (eol() && !other.empty())
      {
         token.set_identifier(other, token_t::case_t::LOWER);
         token.set_type(token_class(other));
         return token;
      }

      last_symbol = symbol;
   }

   return make_token(tell() + get_exp_pos(), data()); // empty token
}


/* -------------------------------------------------------------------------- */

} // namespace nu


