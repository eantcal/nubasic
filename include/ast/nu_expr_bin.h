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
#include "nu_global_function_tbl.h"
#include "nu_proc_scope.h"
#include "nu_var_scope.h"


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
    variant_t eval(rt_prog_ctx_t& ctx) const override
    {
        return _func(_var1->eval(ctx), _var2->eval(ctx));
    }

    //! Returns false for a binary expression
    bool empty() const noexcept override { return false; }

    std::string name() const noexcept override
    {
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

    func_args_t get_args() const noexcept override
    {
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

        if (var_idx.size() > 1 || member_element.empty()) {
            throw exception_t(std::string("Cannot resolve struct element"));
        }

        size_t var_vec_idx = 0;
        if (!var_idx.empty()) {
            var_vec_idx = size_t(var_idx[0]->eval(ctx).to_int());
        }

        // MyBase.Method(args) in expression context:
        // Retrieve Me from the current scope, find the base class from the
        // inheritance table, and dispatch to base_class.method via run_method.
        if (var_name == "mybase") {
            auto local_scope = ctx.proc_scope.get();
            if (local_scope && local_scope->is_defined("me")) {
                const variant_t me_val = (*local_scope)["me"].first;
                if (me_val.is_struct()) {
                    const std::string derived = me_val.struct_type_name();
                    const auto base_it = ctx.class_bases.find(derived);
                    if (base_it != ctx.class_bases.end()) {
                        const std::string mangled
                            = base_it->second + "." + member_element;
                        if (ctx.function_tbl.count(mangled) > 0) {
                            return ctx.program().run_method(
                                mangled, _var2->get_args(), "me", me_val);
                        }
                    }
                }
            }
            throw exception_t(
                std::string("Cannot resolve MyBase.") + member_element);
        }

        // Check if member_element is a method (Function) on the object's class.
        // If so, dispatch via run_method which sets up Me correctly.
        {
            var_scope_t::handle_t obj_scope;
            variant_t obj_value;
            bool obj_found = false;

            const auto dot = var_name.find('.');
            if (dot == std::string::npos) {
                // Simple variable like "g"
                auto st = ctx.proc_scope.get_type(var_name);
                if (st != proc_scope_t::type_t::UNDEF) {
                    obj_scope = ctx.proc_scope.get(st);
                    if (obj_scope && obj_scope->is_defined(var_name)) {
                        obj_value = (*obj_scope)[var_name].first;
                        obj_found = true;
                    }
                }
            } else {
                // Nested path like "seg.a"
                const std::string root = var_name.substr(0, dot);
                auto st = ctx.proc_scope.get_type(root);
                if (st != proc_scope_t::type_t::UNDEF) {
                    obj_scope = ctx.proc_scope.get(st);
                    auto* ptr = ctx.get_struct_member_value(
                        var_name, obj_scope, var_vec_idx);
                    if (ptr) {
                        obj_value = *ptr;
                        obj_found = true;
                    }
                }
            }

            if (obj_found && obj_value.is_struct()) {
                const std::string class_name = obj_value.struct_type_name();
                const std::string mangled = class_name + "." + member_element;
                if (ctx.function_tbl.count(mangled) > 0) {
                    // Method call: dispatch with Me injected
                    return ctx.program().run_method(
                        mangled, _var2->get_args(), var_name, obj_value);
                }
            }

            // Check for static method call in expression context:
            // ClassName.StaticMethod(args) where ClassName is not a variable.
            if (!obj_found) {
                const std::string static_key = var_name + "." + member_element;
                if (ctx.class_static_methods.count(static_key) > 0) {
                    ctx.program().run(static_key, _var2->get_args());
                    // Retrieve function return value (empty for Sub)
                    auto it = ctx.function_retval_tbl.find(static_key);
                    if (it == ctx.function_retval_tbl.end()
                        || it->second.empty())
                        return variant_t();
                    auto ret = it->second.front();
                    it->second.pop_front();
                    if (it->second.empty())
                        ctx.function_retval_tbl.erase(it);
                    return ret;
                }
            }
        }

        // Field access path
        const auto var_member_idx = _var2->get_args();
        if (var_member_idx.size() > 1) {
            throw exception_t(std::string("Cannot resolve struct element"));
        }

        size_t element_vec_idx = 0;
        if (!var_member_idx.empty()) {
            element_vec_idx = size_t(var_member_idx[0]->eval(ctx).to_int());
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

} // namespace nu


/* -------------------------------------------------------------------------- */
