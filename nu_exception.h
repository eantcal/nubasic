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

#ifndef __NU_EXCEPTION_H__
#define __NU_EXCEPTION_H__


/* -------------------------------------------------------------------------- */

#include <exception>
#include <string>
#include "nu_string_tool.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class exception_t : public std::exception
{
public:
   exception_t() = delete;
   exception_t(const exception_t&) = default;
   exception_t& operator=( const exception_t& ) = default;

   /** ctor
     *  @param message C-style string error message.
     */
   explicit exception_t(const char* message) :
      _msg(message)
   {
   }

   //! move ctor
   exception_t(exception_t&& e)
   {
      _msg = std::move(e._msg);
   }

   //! move assign operator
   exception_t& operator=( exception_t&& e )
   {
      if ( &e != this )
         _msg = std::move(e._msg);

      return *this;
   }

   /** ctor
    *  @param message The error message.
    */
   explicit exception_t(const std::string& message) :
      _msg(message)
   {}

   /** dtor
    * Virtual to allow for subclassing.
    */
   virtual ~exception_t() throw ( ) {}

   /** Returns a pointer to the (constant) error description.
    *  @return A pointer to a const char*.
    *          The underlying memory is in posession of the exception_t
    *          object. Callers must not free the memory.
    */
   virtual const char* what() const throw ( ) override
   {
      return _msg.c_str();
   }

protected:
   /** Error message.
    */
   std::string _msg;
};


/* -------------------------------------------------------------------------- */

struct error_code_t
{
   virtual int get() const NU_NOEXCEPT = 0;

   operator int() const NU_NOEXCEPT
   {
      return get();
   }
};


/* -------------------------------------------------------------------------- */

//! Runtime error exception
class runtime_error_t : public exception_t
{
public:
   runtime_error_t() = delete;

   runtime_error_t(const runtime_error_t&) = default;
   runtime_error_t& operator=( const runtime_error_t& ) = default;

   //! ctor
   runtime_error_t(
      int line,
      const error_code_t& error_code,
      const std::string& msg = "")
      :
      exception_t(""),
      _error_code(error_code),
      _line_num(line)
   {
      _msg = ( msg.empty() ? "" : ( ": " + msg ) );
   }

   //! Return error code
   int get_error_code() const NU_NOEXCEPT
   {
      return _error_code;
   }

   //! Return error code
   int get_line_num() const NU_NOEXCEPT
   {
      return _line_num;
   }

   //! Throw an error if cond is true
   static void error_if(bool cond, const runtime_error_t& err)
   {
      if ( cond )
         throw err;
   }

private:
   int _error_code = 0;
   int _line_num = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_EXCEPTION_H__
