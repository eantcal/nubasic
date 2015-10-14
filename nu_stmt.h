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

#ifndef __NU_STMT_H__
#define __NU_STMT_H__


/* -------------------------------------------------------------------------- */

#include "nu_stdtype.h"
#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_error_codes.h"

#include <memory>
#include <list>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class prog_ctx_t;


/* -------------------------------------------------------------------------- */

/**
 * This is the base class of a hierarchy of statement classes.
 */
class stmt_t
{
public:
   stmt_t() = delete;
   stmt_t(const stmt_t&) = delete;
   stmt_t& operator=( const stmt_t& ) = delete;
   stmt_t(prog_ctx_t & ctx) NU_NOEXCEPT;


   int get_stmt_id() const NU_NOEXCEPT
   {
      return _stmt_id;
   }


   enum class stmt_cl_t
   {
      NA,
      EMPTY,
      BLOCK_OF_CODE,
      WHILE_BEGIN,
      WHILE_END,
      FOR_BEGIN,
      FOR_END,
      IF_BLOCK_BEGIN,
      IF_BLOCK_END,
      SUB_BEGIN,
      SUB_END,
      DO_BEGIN,
      DO_END,
      ELSE
   };

   using handle_t = std::shared_ptr < stmt_t > ;

   //! Executes the statement
   virtual void run(rt_prog_ctx_t&) = 0;

   //! Identify the class of statement
   virtual stmt_cl_t get_cl() const NU_NOEXCEPT;

   //dtor
   virtual ~stmt_t();

private:
   int _stmt_id = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_H__