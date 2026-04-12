//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_MYBASE_CALL_H__
#define __NU_STMT_MYBASE_CALL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_pointer.h"
#include "nu_stmt.h"

#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

// Executes a MyBase.Method(args) call from inside a derived-class method.
// _base_class is resolved at parse time from the class_bases table.
// At runtime: retrieves Me from the current (callee) scope and dispatches
// to base_class.method_name with Me injected, then writes Me back.
class stmt_mybase_call_t : public stmt_t {
public:
    stmt_mybase_call_t(const stmt_mybase_call_t&) = delete;
    stmt_mybase_call_t& operator=(const stmt_mybase_call_t&) = delete;
    stmt_mybase_call_t() = delete;

    // No-args version
    stmt_mybase_call_t(const std::string& base_class,
        const std::string& method_name, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _base_class(base_class)
        , _method_name(method_name)
    {
    }

    // With arg list
    stmt_mybase_call_t(const arg_list_t& args, const std::string& base_class,
        const std::string& method_name, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
        , _base_class(base_class)
        , _method_name(method_name)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    std::string _base_class;
    std::string _method_name;
};


/* -------------------------------------------------------------------------- */
} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_MYBASE_CALL_H__
