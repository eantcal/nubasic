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

#ifndef __NU_HXXX_API_H__
#define __NU_HXXX_API_H__


/* -------------------------------------------------------------------------- */

#include "nu_interpreter.h"
#include "nu_global_function_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t hash_set(rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_get(rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_del(rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_chk(rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_cnt(rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);


/* -------------------------------------------------------------------------- */

}

#endif // !__NU_HASH_MODULE_H__
