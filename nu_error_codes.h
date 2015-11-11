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

#ifndef __NU_BASIC_ERR_DEFS_H__
#define __NU_BASIC_ERR_DEFS_H__


/* -------------------------------------------------------------------------- */

#include <string>
#include <map>
#include "nu_exception.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

// Run-time error handling

class rt_error_code_t
{
public:

   enum value_t
   {
      E_INTERNAL,
      E_DIV_BY_ZERO,
      E_WRG_NUM_ARGS,
      E_INVALID_ARGS,
      E_SUB_UNDEF,
      E_FUNC_UNDEF,
      E_TYPE_MISMATCH,
      E_TYPE_ILLEGAL,
      E_VAR_REDEF,
      E_VAR_UNDEF,
      E_INV_VECT_SIZE,
      E_INV_IDENTIF,
      E_NO_MATCH_IF,
      E_NO_MATCH_SUB,
      E_ELSE_DUP_IN_IFBLK,
      E_ELSE_OUTSIDE_IFBLK,
      E_NO_END_IF,
      E_NO_MATCH_FUNC,
      E_NO_RET_VAL,
      E_EXIT_OUTSIDE_FORBLK,
      E_NO_LABEL,
      E_LABEL_NOT_DEF,
      E_IMPL_CNT_NOT_ALLOWED,
      E_FOR_CNT_NOT_DEF,
      E_EXIT_SUB_OUTSIDE_SUB,
      E_MISSING_END_SUB,
      E_EXIT_WHILE_OUT,
      E_NO_MATCH_WEND,
      E_VEC_IDX_OUT_OF_RANGE,
      E_NEXT_WITHOUT_FOR,
      E_NO_MATCH_WHILE,
      E_CANNOT_MOD_CONST,
      E_NO_MATCH_DO,
      E_EXIT_LOOP_OUT,
      E_VAL_OUT_OF_RANGE,
      E_EXIT_FOR_OUT,
      E_NO_MATCH_FOR,
      E_STRUCT_UNDEF,
      E_NO_MATCH_STRUCT,
   };


   void throw_exc(int line, value_t errcode, const std::string& stmt);


   inline
   void throw_if(bool cond, int line, value_t errcode, const std::string& stmt)
   {
      if (cond)
         throw_exc(line, errcode, stmt);
   }


   std::string message(value_t);


   static rt_error_code_t& get_instance() NU_NOEXCEPT;

private:
   static rt_error_code_t * _instance_ptr;

   rt_error_code_t();
   rt_error_code_t(const rt_error_code_t&) = delete;
   rt_error_code_t& operator=(const rt_error_code_t&) = delete;

   std::map<int, std::string> _err_desc_tbl;
};


/* -------------------------------------------------------------------------- */


void syntax_error(const std::string& expr, size_t pos, const std::string& msg = "");


/* -------------------------------------------------------------------------- */

void syntax_error_if(bool cond, const std::string& msg);


/* -------------------------------------------------------------------------- */

void syntax_error_if(
   bool cond,
   const std::string& expr,
   size_t pos,
   const std::string& msg = "");

/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
