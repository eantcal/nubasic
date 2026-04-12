//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */
#pragma once

/* -------------------------------------------------------------------------- */

#include "nu_global_function_tbl.h"
#include "nu_interpreter.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t hash_set(
    rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_get(
    rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_del(
    rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_chk(
    rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);
variant_t hash_cnt(
    rt_prog_ctx_t& ctx, const std::string& fname, const func_args_t& args);


/* -------------------------------------------------------------------------- */

} // namespace nu
