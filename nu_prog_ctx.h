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

#ifndef __NU_PROG_CTX_H__
#define __NU_PROG_CTX_H__


/* -------------------------------------------------------------------------- */

#include "nu_var_scope.h"
#include "nu_proc_scope.h"
#include "nu_symbol_map.h"
#include "nu_label_tbl.h"
#include "nu_prog_pointer.h"
#include "nu_flag_map.h"
#include "nu_instrblock_metadata.h"
#include "nu_proc_prototype_tbl.h"
#include "nu_for_loop_rtdata.h"
#include "nu_file_dscrptr_tbl.h"

#include <memory>
#include <deque>
#include <sstream>
#include <algorithm>
#include <list>
#include <set>
#include <stack>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class prog_ctx_t
{
public:
   prog_ctx_t() = delete;
   prog_ctx_t(const prog_ctx_t&) = delete;
   prog_ctx_t& operator=( const prog_ctx_t& ) = delete;

   virtual ~prog_ctx_t() {}

   prog_pointer_t::stmt_number_t make_next_stmt_id() NU_NOEXCEPT
   {
      // first valid id is 1
      return ++_stmt_id_cnt;
   }

   //Enable/disable debug tracing
   bool tracing_on = false;

   //Procedure prototypes
   proc_prototype_tbl_t proc_prototypes;
   std::set<std::string> function_tbl;

   //Structure prototypes
   struct_prototype_tbl_t struct_prototypes;

   //Procedure variable space table
   proc_scope_t proc_scope;

   //Program counter updated at compile time
   prog_pointer_t compiletime_pc;

   //Program label table
   label_tbl_t prog_label;

   //FOR-Loop metadata (created compiling BASIC source code)
   instrblock_metadata_t for_loop_metadata;

   //WHILE-Loop metadata (created compiling BASIC source code)
   instrblock_metadata_t while_metadata;

   //DO-LOOP-WHILE metadata (created compiling BASIC source code)
   instrblock_metadata_t do_loop_while_metadata;

   //IF-statement metadata (created compiling source code)
   if_instrblock_metadata_t if_metadata;

   //Procedure metadata (created compiling BASIC source code)
   instrblock_metadata_t procedure_metadata;

   //Struct metadata (created compiling BASIC source code)
   instrblock_metadata_t struct_metadata;
   std::string compiling_struct_name;


   prog_ctx_t(FILE * stdout_ptr, FILE * stdin_ptr);

   void clear_metadata();

   //Print-out traces of control-structures and procedure metadata
   void trace_metadata(std::stringstream& ss);

   //Get stdout file pointer
   FILE * get_stdout_ptr() const NU_NOEXCEPT
   {
      return _stdout_ptr;
   }

   //Get stdin file pointer
   FILE * get_stdin_ptr() const NU_NOEXCEPT
   {
      return _stdin_ptr;
   }

private:
   prog_pointer_t::stmt_number_t _stmt_id_cnt = 0;

   // STD I/O file pointers
   FILE * _stdout_ptr = stdout;
   FILE * _stdin_ptr = stdin;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#define NU_TRACE_CTX_AUX(__CTX, _DBGINFO) do {\
    std::stringstream ss; \
    (__CTX).trace_rtdata(ss); \
    if (_DBGINFO)\
        fprintf((__CTX).get_stdout_ptr(), "\nNU_TRACE_CTX\n%s\n", __FUNCTION__); \
    fprintf((__CTX).get_stdout_ptr(), "%s\n", ss.str().c_str()); \
} while (0)



/* -------------------------------------------------------------------------- */

#define NU_TRACE_CTX(__CTX) do {\
if ((__CTX).tracing_on)\
{\
    NU_TRACE_CTX_AUX(__CTX, false);\
}\
} while (0)


/* -------------------------------------------------------------------------- */

#endif // __NU_PROG_CTX_H__
