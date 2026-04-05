//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_GENINSTR_H__
#define __NU_STMT_GENINSTR_H__


/* -------------------------------------------------------------------------- */

#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_stmt.h"

#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_ctx_t;
class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

//
// int f(T args[ARGSNUM])
//
template <typename F, typename T, int ARGSNUM>
class stmt_geninstr_t : public stmt_t {
public:
    using data_t = T;

    stmt_geninstr_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_geninstr_t(arg_list_t args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
    }


protected:
    template <class DT> void run_aux(rt_prog_ctx_t& ctx, std::true_type)
    {
        std::vector<data_t> f_args;

        for (auto arg : _args) {
            data_t val = data_t(arg.first->eval(ctx));
            f_args.push_back(val);
        }

        ctx.set_errno(F()(ctx, f_args));
    }


    template <class DT> void run_aux(rt_prog_ctx_t& ctx, std::false_type)
    {
        std::vector<data_t> f_args;

        for (auto arg : _args) {
            f_args.push_back(arg.first->eval(ctx));
        }

        ctx.set_errno(F()(ctx, f_args));
    }


public:
    void run(rt_prog_ctx_t& ctx)
    {
        // if number of arguments is negative
        // do not check it
        if (ARGSNUM >= 0) {
            syntax_error_if(
                _args.size() < ARGSNUM, "Instruction expects to be passed "
                    + nu::to_string(ARGSNUM) + " arguments");
        }

        run_aux<T>(ctx, std::is_integral<T>());

        ctx.go_to_next();
    }

protected:
    arg_list_t _args;
    stmt_geninstr_t(const stmt_geninstr_t&) = delete;
    stmt_geninstr_t& operator=(const stmt_geninstr_t&) = delete;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_GENINSTR_H__
