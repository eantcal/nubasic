//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_BIN_H__
#define __NU_EXPR_BIN_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_global_function_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class expr_bin_t : public expr_any_t {
public:
    using func_t = func_bin_t;

    //! ctor
    expr_bin_t(func_t f, expr_any_t::handle_t var1, expr_any_t::handle_t var2)
        : _func(f)
        , _var1(var1)
        , _var2(var2)
    {
    }


    expr_bin_t() = delete;
    expr_bin_t(const expr_bin_t&) = default;
    expr_bin_t& operator=(const expr_bin_t&) = default;

    //! Returns f(var1, var2) appling ctor given arguments
    variant_t eval(rt_prog_ctx_t& ctx) const override {
        return _func(_var1->eval(ctx), _var2->eval(ctx));
    }

    //! Returns false for a binary expression
    bool empty() const noexcept override { 
        return false; 
    }

    std::string name() const noexcept override {
        std::string ret;

        if (_var1) {
            ret = _var1->name();

            if (_var2 && !_var2->name().empty()) {
                ret += ".";
                ret += _var2->name();
            }
        }

        return ret;
    }

    func_args_t get_args() const noexcept override {
        func_args_t ret;

        if (_var1) {
            ret = _var1->get_args();
        }

        if (_var2) {
            for (const auto& e : _var2->get_args())
                ret.push_back(e);
        }

        return ret;
    }


protected:
    func_bin_t _func;
    expr_any_t::handle_t _var1, _var2;
};


/* -------------------------------------------------------------------------- */

class expr_struct_access_t : public expr_bin_t {
public:
    using func_t = func_bin_t;

    //! ctor
    expr_struct_access_t(expr_any_t::handle_t var1, expr_any_t::handle_t var2)
        : expr_bin_t(nu::func_bin_t(), var1, var2)
    {
    }

    expr_struct_access_t() = delete;
    expr_struct_access_t(const expr_struct_access_t&) = default;
    expr_struct_access_t& operator=(const expr_struct_access_t&) = default;

    //! Returns f(var1, var2) appling ctor given arguments
    variant_t eval(rt_prog_ctx_t& ctx) const override
    {
        if (!_var1 || !_var2) {
            throw exception_t(std::string("Cannot resolve struct element"));
        }

        const auto var_name = _var1->name();
        const auto var_idx = _var1->get_args();

        const auto member_element = _var2->name();
        const auto var_member_idx = _var2->get_args();

        if (var_idx.size() > 1 || var_member_idx.size() > 1
            || member_element.empty())
        {
            throw exception_t(std::string("Cannot resolve struct element"));
        }

        size_t var_vec_idx = 0;
        size_t element_vec_idx = 0;

        if (!var_idx.empty()) {
            var_vec_idx = size_t(var_idx[0]->eval(ctx).to_long64());
        }

        if (!var_member_idx.empty()) {
            element_vec_idx = size_t(var_member_idx[0]->eval(ctx).to_long64());
        }

        std::string err;

        const variant_t res = ctx.resolve_struct_element(
            var_name, var_vec_idx, member_element, element_vec_idx, err);

        if (!err.empty()) {
            throw exception_t(err);
        }

        return res;
    }
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
