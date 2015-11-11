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

#include "nu_error_codes.h"
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

rt_error_code_t * rt_error_code_t::_instance_ptr = nullptr;


/* -------------------------------------------------------------------------- */

void rt_error_code_t::throw_exc(
   int line,
   value_t errcode,
   const std::string& stmt)
{
   struct err_t : public error_code_t
   {
      int _err = 0;

      err_t(int err) : _err(err) {}

      int get() const NU_NOEXCEPT
      {
         return _err;
      }
   };

   std::string errmsg = stmt.empty() ? "" : stmt + " ";
   errmsg += rt_error_code_t::get_instance().message(errcode);

   err_t err(errcode);

   runtime_error_t::error_if(
      true,
      runtime_error_t(line, err, errmsg));
}


/* -------------------------------------------------------------------------- */

std::string rt_error_code_t::message(value_t err_code)
{
   std::string ret;

   auto err_it = _err_desc_tbl.find(err_code);

   if (err_it != _err_desc_tbl.end())
      ret = err_it->second;

   return ret;
}


/* -------------------------------------------------------------------------- */

rt_error_code_t& rt_error_code_t::get_instance() NU_NOEXCEPT
{
   if (!_instance_ptr)
      _instance_ptr = new rt_error_code_t();

   assert(_instance_ptr);

   return *_instance_ptr;
}


/* -------------------------------------------------------------------------- */

rt_error_code_t::rt_error_code_t()
{
   auto & e = _err_desc_tbl;
   e[E_INTERNAL] = "invalid metadata. Try to rebuild";
   e[E_DIV_BY_ZERO] = "division by zero";
   e[E_WRG_NUM_ARGS] = "wrong number of arguments";
   e[E_INVALID_ARGS] = "invalid arguments";
   e[E_SUB_UNDEF] = "procedure not defined";
   e[E_FUNC_UNDEF] = "function not defined";
   e[E_TYPE_MISMATCH] = "type mismatch";
   e[E_TYPE_ILLEGAL] = "type illegal";
   e[E_VAR_REDEF] = "variable already defined";
   e[E_VAR_UNDEF] = "variable not defined";
   e[E_INV_VECT_SIZE] = "invalid vector size";
   e[E_INV_IDENTIF] = "invalid identifier";
   e[E_ELSE_OUTSIDE_IFBLK] = "ELSE outside IF block";
   e[E_ELSE_DUP_IN_IFBLK] = "ELSE already defined in this IF block";
   e[E_NO_END_IF] = "missing END IF statement";
   e[E_NO_MATCH_IF] = "matching IF statement not found";
   e[E_NO_MATCH_SUB] = "matching SUB statement not found";
   e[E_NO_MATCH_WHILE] = "matching WHILE statement not found";
   e[E_NO_MATCH_DO] = "matching DO statement not found";
   e[E_NO_MATCH_FUNC] = "END FUNCTION without maching FUNCTION";
   e[E_NO_RET_VAL] = "missing function return value";
   e[E_EXIT_OUTSIDE_FORBLK] = "EXIT FOR outside FOR loop";
   e[E_NO_LABEL] = "missing label";
   e[E_LABEL_NOT_DEF] = "label not defined";
   e[E_IMPL_CNT_NOT_ALLOWED] = "implicit counter not allowed in nested loops";
   e[E_EXIT_SUB_OUTSIDE_SUB] = "EXIT SUB/FUNCTION outside procedure";
   e[E_MISSING_END_SUB] = "missing END SUB/FUNCTION statement";
   e[E_EXIT_WHILE_OUT] = "EXIT WHILE outside WHILE loop";
   e[E_EXIT_LOOP_OUT] = "EXIT DO outside DO-LOOP-WHILE block";
   e[E_EXIT_FOR_OUT] = "EXIT FOR outside FOR-NEXT block";
   e[E_NO_MATCH_WEND] = "matching WEND/END WHILE not found";
   e[E_VEC_IDX_OUT_OF_RANGE] = "array index out of range";
   e[E_NEXT_WITHOUT_FOR] = "NEXT without FOR";
   e[E_CANNOT_MOD_CONST]= "cannot modify a constant value";
   e[E_VAL_OUT_OF_RANGE] = "value out of range";
   e[E_NO_MATCH_FOR] = "matching FOR statement not found";
   e[E_STRUCT_UNDEF] = "STRUCT not defined";
   e[E_NO_MATCH_STRUCT] = "matching STRUCT statement not defined";
}


/* -------------------------------------------------------------------------- */

void syntax_error(
   const std::string& expr,
   size_t pos,
   const std::string& msg)
{
   std::string err(msg.empty() ? "Syntax Error" : msg);
   err += " at (" + nu::to_string(pos + 1) + "):\n";

   if (expr.size() >pos)
   {
      err += expr + "\n";

      for (size_t i = 0; i < pos; ++i) err += " ";

      for (size_t i = 0; i < expr.size()-pos; ++i) err += "^";
   }

   throw exception_t(err);
}


/* -------------------------------------------------------------------------- */

void syntax_error_if(
   bool condition,
   const std::string& expr,
   size_t pos,
   const std::string& msg)
{
   if (condition)
      syntax_error(expr, pos, msg);
}


/* -------------------------------------------------------------------------- */

void syntax_error_if(bool condition, const std::string& msg)
{
   if (condition)
      throw exception_t(msg);
}


/* -------------------------------------------------------------------------- */

}

