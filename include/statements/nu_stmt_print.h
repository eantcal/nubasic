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

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_print_t : public stmt_t {
public:
    stmt_print_t(const stmt_print_t&) = delete;
    stmt_print_t& operator=(const stmt_print_t&) = delete;

    stmt_print_t(prog_ctx_t& ctx, int fd = 0, bool unicode = false,
        const std::string& data = "", bool suppress_final_newline = false)
        : stmt_t(ctx)
        , _fd(fd)
        , _unicode(unicode)
        , _data(data)
        , _suppress_final_newline(suppress_final_newline)
    {
    }

    //! Used by parse_arg_list when the argument list is empty (no expressions).
    stmt_print_t(
        prog_ctx_t& ctx, int fd, bool unicode, bool suppress_final_newline)
        : stmt_t(ctx)
        , _fd(fd)
        , _unicode(unicode)
        , _suppress_final_newline(suppress_final_newline)
    {
    }

    stmt_print_t(const arg_list_t& args, prog_ctx_t& ctx, int fd, bool unicode,
        bool suppress_final_newline = false)
        : stmt_t(ctx)
        , _args(args)
        , _fd(fd)
        , _unicode(unicode)
        , _suppress_final_newline(suppress_final_newline)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    int _fd = 0; /* 0 means 'use stdout' */
    bool _unicode = false;
    std::string _data;
    //! If true (Write stmt): no implicit newline after the last expression.
    bool _suppress_final_newline = false;
};


/* -------------------------------------------------------------------------- */
} // namespace nu


/* -------------------------------------------------------------------------- */
