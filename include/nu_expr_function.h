//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_FUNCTION_H__
#define __NU_EXPR_FUNCTION_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_global_function_tbl.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

//! This class represents a function-call expression
class expr_function_t : public expr_any_t {
public:
    //! ctor
    expr_function_t(const std::string& name, func_args_t var) noexcept
        : _name(name),
          _var(var)
    {
    }

    expr_function_t() = delete;
    expr_function_t(const expr_function_t&) = default;
    expr_function_t& operator=(const expr_function_t&) = default;

    //! Evaluates the function (using name and arguments given to the ctor)
    variant_t eval(rt_prog_ctx_t& ctx) const override;

    //! Returns false for this type of object
    bool empty() const noexcept override;

    std::string name() const noexcept override { return _name; }

    func_args_t get_args() const noexcept override { return _var; }


protected:
    std::string _name;
    func_args_t _var;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_FUNCTION_H__
