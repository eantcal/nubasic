//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "nu_error_codes.h"
#include "nu_expr_parser.h"
#include "nu_global_function_tbl.h"
#include "nu_rt_prog_ctx.h"
#include "nu_stmt_let.h"
#include "nu_tokenizer.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t eval_expr(rt_prog_ctx_t& ctx, std::string data)
{
    try {
        nu::token_list_t tl;
        tokenizer_t st(data);
        expr_parser_t ep;

        auto h = ep.compile(st);
        return h->eval(ctx);
    } 
    catch (nu::exception_t&) {
        return variant_t(std::string("Syntax Error"));
    } 
    catch (std::exception&) {
        return variant_t(std::string("Runtime Error"));
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
