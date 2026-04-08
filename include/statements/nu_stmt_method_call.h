//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_METHOD_CALL_H__
#define __NU_STMT_METHOD_CALL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_pointer.h"
#include "nu_stmt.h"

#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

// Executes an object method call: obj.Method(args)
// At runtime resolves the object's class, checks access, and dispatches.
class stmt_method_call_t : public stmt_t {
public:
    stmt_method_call_t(const stmt_method_call_t&) = delete;
    stmt_method_call_t& operator=(const stmt_method_call_t&) = delete;
    stmt_method_call_t() = delete;

    // No-args constructor (used when arg list is empty)
    stmt_method_call_t(const std::string& obj_name,
        const std::string& method_name, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _obj_name(obj_name)
        , _method_name(method_name)
    {
    }

    // Constructor with arg list (used by parse_arg_list)
    stmt_method_call_t(const arg_list_t& args, const std::string& obj_name,
        const std::string& method_name, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
        , _obj_name(obj_name)
        , _method_name(method_name)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    std::string _obj_name;
    std::string _method_name;
};


/* -------------------------------------------------------------------------- */
} // namespace nu


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_METHOD_CALL_H__
