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

#ifndef __NU_STDLIB_H__
#define __NU_STDLIB_H__


/* -------------------------------------------------------------------------- */

#include "nu_generic_functor.h"

#include <string>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

//! Executes a shell command
struct os_shell_t : public generic_functor_t
{
   static int exec(const std::string& cmd);

   virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Changes current working directory
struct os_chdir_t : public generic_functor_t
{
   virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements FOPEN
struct os_fopen_t : public generic_functor_t
{
   virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements SEEK
struct os_fseek_t : public generic_functor_t
{
   virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements FLUSH
struct os_fflush_t : public generic_functor_t
{
   virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements READ
struct os_fread_t : public generic_functor_t
{
   virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STDLIB_H__