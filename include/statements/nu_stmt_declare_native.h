//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#pragma once

#include "nu_native_decl.h"
#include "nu_prog_ctx.h"
#include "nu_stmt.h"

namespace nu {

struct stmt_declare_native_t : public stmt_t {
    stmt_declare_native_t(prog_ctx_t& ctx, native_function_decl_t declaration);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override { return stmt_cl_t::NA; }
    bool is_debug_steppable() const noexcept override { return false; }

private:
    native_function_decl_t _declaration;
};

} // namespace nu
