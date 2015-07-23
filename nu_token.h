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

#ifndef __NU_TOKEN_H__
#define __NU_TOKEN_H__


/* -------------------------------------------------------------------------- */

#include <string>
#include <memory>
#include <algorithm>
#include <deque>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

//! Tonken class indentifier
enum class tkncl_t
{
   UNDEFINED,
   BLANK,
   NEWLINE,
   IDENTIFIER,
   INTEGRAL,
   REAL,
   OPERATOR,
   SUBEXP_BEGIN,
   SUBEXP_END,
   STRING_LITERAL,
   STRING_COMMENT,
   SUBSCR_BEGIN,
   SUBSCR_END,
   LINE_COMMENT
};


/* -------------------------------------------------------------------------- */

/**
 * This class holds a token data
 */
class token_t
{
   friend class base_tknzr_t;
public:
   enum class case_t { LOWER, NOCHANGE };

   using data_ptr_t = std::shared_ptr< std::string >;

   token_t(
      const std::string& id,
      tkncl_t t,
      size_t pos,
      data_ptr_t expr_ptr) throw();

   token_t() = delete;
   token_t(const token_t&) = default;
   token_t& operator=(const token_t&) = default;

   void set_identifier(const std::string& id, case_t casemode);

   const std::string& identifier() const throw()
   {
      return _identifier;
   }

   const std::string& org_id() const throw()
   {
      return _org_id;
   }

   tkncl_t type() const throw()
   {
      return _type;
   }

   void set_type(tkncl_t cl) throw()
   {
      _type = cl;
   }

   size_t position() const throw()
   {
      return _position;
   }

   void set_position(size_t pos) throw()
   {
      _position = pos;
   }

   std::string expression() const throw()
   {
      return *_expression_ptr;
   }

   data_ptr_t expression_ptr() const throw()
   {
      return _expression_ptr;
   }

   token_t(token_t&& other) :
      _identifier(std::move(other._identifier)),
      _org_id(std::move(other._org_id)),
      _type(std::move(other._type)),
      _position(std::move(other._position)),
      _expression_ptr(std::move(other._expression_ptr))
   {
   }

   token_t& operator=(token_t&& other)
   {
      if (this != &other)
      {
         _identifier = std::move(other._identifier);
         _org_id = std::move(other._org_id);
         _type = std::move(other._type);
         _position = std::move(other._position);
         _expression_ptr = std::move(other._expression_ptr);
      }

      return *this;
   }

protected:
   token_t(size_t pos, data_ptr_t expr_ptr) throw()
      :
      _position(pos),
      _expression_ptr(expr_ptr)
   {}

private:
   std::string _identifier;
   std::string _org_id;
   tkncl_t     _type = tkncl_t::UNDEFINED;
   size_t      _position = 0;
   data_ptr_t  _expression_ptr;

   void _set_id_lowercase()
   {
      std::transform(
         _identifier.begin(),
         _identifier.end(),
         _identifier.begin(),
         tolower);
   }

public:
   static std::string description_of(tkncl_t tc);
   friend std::ostream& operator<<(std::ostream& os, const nu::token_t& t);
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_TOKEN_H__
