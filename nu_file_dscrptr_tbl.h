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

#ifndef __NU_FILE_DSCRPTR_TBL_H__
#define __NU_FILE_DSCRPTR_TBL_H__


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

#include <memory>
#include <deque>
#include <sstream>
#include <algorithm>
#include <list>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

struct file_dscrptr_t
{
   friend struct file_dscrptr_tbl_t;

   using handle_t = std::shared_ptr < file_dscrptr_t > ;

   file_dscrptr_t() = delete;
   FILE* data() const NU_NOEXCEPT;

   ~file_dscrptr_t();
   file_dscrptr_t(FILE * fptr) : _fptr(fptr) {}

protected:
   FILE* _fptr = nullptr;
   bool close() NU_NOEXCEPT;
};


/* -------------------------------------------------------------------------- */

struct file_dscrptr_tbl_t
{
   bool open_fd(
      const std::string& fname, // file path
      const std::string& fmode, // see fopen modes
      unsigned int fd);

   FILE* resolve_fd(unsigned int fd);
   bool close_fd(unsigned int fd);
   bool seek_fd(int seek_ptr, int seek_origin, unsigned int fd);
   bool flush_fd(unsigned int fd);
   void trace(std::stringstream & ss);

   bool empty() const NU_NOEXCEPT
   {
      return _file_tbl.empty();
   }

   void clear() NU_NOEXCEPT
   {
      _file_tbl.clear();
   }


protected:
   //File Tbl: <Key:id, Value:file-ctx-handle>
   std::map<int, file_dscrptr_t::handle_t> _file_tbl;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_FILE_DSCRPTR_TBL_H__
