//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_UNARY_OP_H__
#define __NU_EXPR_UNARY_OP_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class expr_unary_op_t : public expr_any_t {
public:
    expr_unary_op_t(const std::string& op_name, expr_any_t::handle_t var);

    expr_unary_op_t() = delete;
    expr_unary_op_t(const expr_unary_op_t&) = default;
    expr_unary_op_t& operator=(const expr_unary_op_t&) = default;

    variant_t eval(rt_prog_ctx_t& ctx) const override;
    bool empty() const noexcept override;

    std::string name() const noexcept override {
        return _var ? _var->name() : "";
    }

    func_args_t get_args() const noexcept override {
        func_args_t dummy;
        return dummy;
    }

protected:
    std::string _op_name;
    expr_any_t::handle_t _var;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_UNARY_OP_H__
