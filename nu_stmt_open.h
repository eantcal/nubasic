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

#ifndef __NU_STMT_OPEN_H__
#define __NU_STMT_OPEN_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_stmt_empty.h"
#include "nu_expr_any.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_open_t : public stmt_t
{
public:
   stmt_open_t() = delete;
   stmt_open_t(const stmt_open_t&) = delete;
   stmt_open_t& operator=(const stmt_open_t&) = delete;

   stmt_open_t(
      prog_ctx_t& ctx,
      expr_any_t::handle_t filename,
      const std::string & mode,
      const std::string & access,
      unsigned int fd)
      :
      stmt_t(ctx),
      _filename(filename),
      _mode(mode),
      _access(access),
      _fd(fd)
   {}

   virtual void run(rt_prog_ctx_t& ctx) override;

protected:
   expr_any_t::handle_t _filename;
   std::string _mode;
   std::string _access;
   unsigned int _fd = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_OPEN_H__