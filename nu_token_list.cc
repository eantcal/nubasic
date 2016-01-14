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

#include "nu_token_list.h"
#include "nu_exception.h"
#include "nu_error_codes.h"

#include <cassert>

/* -------------------------------------------------------------------------- */

namespace nu
{
      
   
/* -------------------------------------------------------------------------- */

//prefix
token_list_t& token_list_t::operator--()
{
   assert(!empty());

   data().erase(begin());

   return *this;
}


/* -------------------------------------------------------------------------- */

//postfix
token_list_t& token_list_t::operator--(int)
{
   assert(!empty());

   data().erase(end() - 1);

   return *this;
}


/* -------------------------------------------------------------------------- */

token_list_t token_list_t::sublist(size_t pos, size_t items)
{
   assert(!((pos + items) > size()));

   token_list_t ret;

   for (size_t i = pos; i < (items + pos); ++i)
   {
      ret += (*this)[i];
   }

   return ret;
}


/* -------------------------------------------------------------------------- */

size_t token_list_t::find(
   std::function<bool(size_t)> test,
   size_t pos,
   size_t items)
{
   if (!items)
      items = size();

   assert(!((pos + items) > size()));

   for (size_t i = pos; i < (pos + items); ++i)
   {
      if (test(i))
         return i;
   }

   return npos;
}


/* -------------------------------------------------------------------------- */

size_t token_list_t::find(const token_t& t, size_t pos, size_t items)
{
   auto test = [&](size_t i)
   {
      return
         (*this)[i].identifier() == t.identifier() &&
         (*this)[i].type() == t.type();
   };

   return find(test, pos, items);
}


/* -------------------------------------------------------------------------- */

size_t token_list_t::find(
   const std::string& identifier, size_t pos, size_t items)
{
   auto test = [&](size_t i)
   {
      return (*this)[i].identifier() == identifier;
   };

   return find(test, pos, items);
}


/* -------------------------------------------------------------------------- */

size_t token_list_t::find(const tkncl_t type, size_t pos, size_t items)
{
   auto test = [&](size_t i)
   {
      return (*this)[i].type() == type;
   };

   return find(test, pos, items);
}


/* -------------------------------------------------------------------------- */

std::list<token_list_t> token_list_t::get_parameters(
   btfunc_t test_begin,
   btfunc_t test_end,
   btfunc_t test_separator,
   size_t search_from)
{
   assert(search_from < size());

   token_list_t ret;
   int level = 0;
   size_t end_pos = 0;
   size_t begin_pos = 0;
   std::list<token_list_t> ret_list;

   for (size_t i = search_from; i < size(); ++i)
   {
      const token_t& t = (*this)[i];

      if (test_begin(t))
      {
         if (level == 0)
            begin_pos = i;

         ++level;
      }
      else if (test_end(t))
      {
         --level;

         if (level < 1)
         {
            ret_list.push_back(ret);
            break;
         }
      }

      if (level == 1 && test_separator(t))
      {
         ret_list.push_back(ret);
         ret.clear();

         if (i == end_pos)
            break;
      }
      else if (level>0)
      {
         if (i != begin_pos)
            ret += t;
      }
   }

   if (level > 0)
   {
      const token_t& t = (*this)[end_pos];
      syntax_error(
         t.expression(), t.position(), "Missing token, error");
   }

   return ret_list;
}


/* -------------------------------------------------------------------------- */

std::list<token_list_t> token_list_t::get_parameters(
   const tkp_t& tbegin,
   const tkp_t& tend,
   const tkp_t& tseparator)
{
   auto fb = [&](const token_t& t)
   {
      return
         t.identifier() == tbegin.first &&
         t.type() == tbegin.second;
   };

   auto fe = [&](const token_t& t)
   {
      return
         t.identifier() == tend.first &&
         t.type() == tend.second;
   };

   auto fs = [&](const token_t& t)
   {
      return
         t.identifier() == tseparator.first &&
         t.type() == tseparator.second;
   };

   return get_parameters(fb, fe, fs, 0);
}


/* -------------------------------------------------------------------------- */

token_list_t token_list_t::sublist(
   std::function<bool(const token_t&)> test_begin,
   std::function<bool(const token_t&)> test_end,
   size_t & search_from,
   bool b_erase)
{
   assert(search_from < size());

   token_list_t ret;
   int level = 0;
   size_t end_pos = 0;
   size_t begin_pos = 0;

   for (size_t i = search_from; i < size(); ++i)
   {
      const token_t& t = (*this)[i];

      if (test_begin(t))
      {
         if (level == 0)
            begin_pos = i;

         ++level;
      }
      else if (test_end(t))
      {
         --level;

         if (level < 1)
         {
            end_pos = i;
            ret += t;
            break;
         }
      }

      if (level>0)
         ret += t;

   }

   if (level > 0)
   {
      const token_t& t = (*this)[end_pos];
      
      syntax_error(
         t.expression(), t.position(), "Missing token, error");
   }

   if (b_erase)
      data().erase( begin() + begin_pos, begin() + end_pos );

   search_from = begin_pos;

   return ret;
}


/* -------------------------------------------------------------------------- */

token_list_t token_list_t::sublist(
   const token_t& first,
   const token_t& second,
   size_t search_from,
   bool b_erase)
{

   auto test_begin = [&](const token_t& t)
   {
      return
         t.identifier() == first.identifier() &&
         t.type() == first.type();
   };

   auto test_end = [&](const token_t& t)
   {
      return
         t.identifier() == second.identifier() &&
         t.type() == second.type();
   };


   return sublist(test_begin, test_end, search_from, b_erase);
}


//-------------------------------------------------------------------------

token_list_t token_list_t::sublist(
   const tkp_t& first,
   const tkp_t& second,
   size_t search_from,
   bool b_erase)
{

   auto test_begin = [&](const token_t& t)
   {
      return
         t.identifier() == first.first &&
         t.type() == first.second;
   };

   auto test_end = [&](const token_t& t)
   {
      return
         t.identifier() == second.first &&
         t.type() == second.second;
   };


   return sublist(test_begin, test_end, search_from, b_erase);
}


/* -------------------------------------------------------------------------- */

token_list_t::data_t::iterator token_list_t::skip_right(
   data_t::iterator search_from,
   const tkp_t& first,
   const tkp_t& second)
{
   assert(search_from != end());

   int level = 0;

   for (; search_from != end(); ++search_from)
   {
      const token_t& t = *search_from;

      if (t.identifier() == first.first && t.type() == first.second)
         ++level;

      if (t.identifier() == second.first &&
            t.type() == second.second)
      {
         --level;

         if (level < 1)
         {
            ++search_from;
            break;
         }
      }
   }

   if (level>0)
   {
      const token_t& t = *(end() - 1);
      
      syntax_error(
         t.expression(), t.position(), "Missing token, error");
   }

   return search_from;
}


/* -------------------------------------------------------------------------- */

token_list_t::data_t::reverse_iterator token_list_t::skip_left(
   data_t::reverse_iterator search_from,
   const tkp_t& first,
   const tkp_t& second)
{
   assert(search_from != rend());

   int level = 0;

   for (; search_from != rend(); ++search_from)
   {
      const token_t& t = *search_from;

      if (t.identifier() == second.first && t.type() == second.second)
         ++level;

      if (t.identifier() == first.first && t.type() == first.second)
      {
         --level;

         if (level < 1)
         {
            ++search_from;
            break;
         }
      }
   }

   if (level>0)
   {
      const token_t& t = *(rend() - 1);
      syntax_error(
         t.expression(), t.position(), "Missing token, error");
   }

   return search_from;
}


/* -------------------------------------------------------------------------- */

token_list_t token_list_t::replace_sublist(
   std::function<bool(const token_t&)> test_begin,
   std::function<bool(const token_t&)> test_end,
   size_t& search_from,
   const token_list_t& replist)
{
   assert(search_from < size());

   token_list_t sub_list = sublist(test_begin, test_end, search_from, false);
   token_list_t head_list = sublist(0, search_from);
   token_list_t tail_list =
      sublist(
         search_from + sub_list.size(),
         size()- search_from - sub_list.size());

   return head_list + replist + tail_list;
}


/* -------------------------------------------------------------------------- */

token_list_t token_list_t::replace_sublist(
   size_t begin_pos,
   size_t end_pos,
   const token_list_t& replist)
{
   assert(!(end_pos < begin_pos || end_pos >= size()));

   token_list_t sub_list = sublist(begin_pos, end_pos - begin_pos + 1);
   token_list_t head_list = sublist(0, begin_pos);
   token_list_t tail_list = sublist(end_pos + 1, size() - end_pos - 1);

   return head_list + replist + tail_list;
}


/* -------------------------------------------------------------------------- */

token_list_t token_list_t::replace_sublist(
   const tkp_t& first,
   const tkp_t& second,
   size_t search_from,
   const token_list_t& replist)
{

   auto test_begin = [&](const token_t& t)
   {
      return
         t.identifier() == first.first &&
         t.type() == first.second;
   };

   auto test_end = [&](const token_t& t)
   {
      return
         t.identifier() == second.first &&
         t.type() == second.second;
   };

   return replace_sublist(test_begin, test_end, search_from, replist);
}


/* -------------------------------------------------------------------------- */

token_t token_list_t::operator[](size_t idx) const
{
   assert(!empty());

   if (idx > size())
   {
      auto i = (cend() - 1);
      syntax_error(i->expression(), i->position());
   }

   return data().operator[](idx);
}


/* -------------------------------------------------------------------------- */

token_t& token_list_t::operator[](size_t idx)
{
   assert(!empty());

   if (idx > size())
   {
      auto i = (cend() - 1);
      syntax_error(i->expression(), i->position());
   }

   return data().operator[](idx);
}


/* -------------------------------------------------------------------------- */

void token_list_t::_chekpos(size_t pos, size_t items)
{
   syntax_error_if((pos + items) > size(), "Internal error 13");
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const nu::token_list_t& tl)
{
   for (const auto& e : tl.data())
   {
      if (e.identifier().empty())
         os << (e.type() == nu::tkncl_t::SUBEXP_BEGIN ? "{" : "}");
      else
         os << e.identifier();
   }

   return os;
}
