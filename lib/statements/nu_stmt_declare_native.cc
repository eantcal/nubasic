//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_stmt_declare_native.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"

#include <utility>

namespace nu {

stmt_declare_native_t::stmt_declare_native_t(
    prog_ctx_t& ctx, native_function_decl_t declaration)
    : stmt_t(ctx)
    , _declaration(std::move(declaration))
{
    std::string error;
    syntax_error_if(!ctx.native_functions.declare(_declaration, &error),
        _declaration.basic_name, 0, error);
}

void stmt_declare_native_t::run(rt_prog_ctx_t& ctx)
{
    if (!ctx.native_calls_enabled) {
        throw exception_t(
            "Native DLL calls are disabled by --disable-native-calls.");
    }

    ctx.native_runtime.resolve(_declaration);
    ctx.go_to_next();
}

} // namespace nu
