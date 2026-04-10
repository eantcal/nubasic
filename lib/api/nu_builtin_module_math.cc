//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"

#include <cmath>
#include <cstdlib>
#include <ctime>

namespace nu {
namespace {

    class math_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "math";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports = { "truncf",
                "sin", "cos", "tan", "log", "log10", "exp", "abs", "asin",
                "acos", "atan", "sinh", "cosh", "tanh", "sqrt", "sign", "min",
                "max", "pow", "int", "sqr", "rnd", "not", "b_not" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
#define __FUNCTOR_BUILDER(_FNC_)                                               \
    struct _##_FNC_ {                                                          \
        float operator()(float x) noexcept                                     \
        {                                                                      \
            return _FNC_(x);                                                   \
        }                                                                      \
    };                                                                         \
    fmap[#_FNC_] = math_functor<float, _##_FNC_>;

            __FUNCTOR_BUILDER(truncf);

#undef __FUNCTOR_BUILDER

#define __DOUBLE_FUNCTOR_BUILDER(_FNC_)                                        \
    struct _##_FNC_ {                                                          \
        double operator()(double x) noexcept                                   \
        {                                                                      \
            return _FNC_(x);                                                   \
        }                                                                      \
    };                                                                         \
    fmap[#_FNC_] = math_functor<double, _##_FNC_>;

            __DOUBLE_FUNCTOR_BUILDER(sin);
            __DOUBLE_FUNCTOR_BUILDER(cos);
            __DOUBLE_FUNCTOR_BUILDER(tan);
            __DOUBLE_FUNCTOR_BUILDER(log);
            __DOUBLE_FUNCTOR_BUILDER(log10);
            __DOUBLE_FUNCTOR_BUILDER(exp);
            __DOUBLE_FUNCTOR_BUILDER(abs);
            __DOUBLE_FUNCTOR_BUILDER(asin);
            __DOUBLE_FUNCTOR_BUILDER(acos);
            __DOUBLE_FUNCTOR_BUILDER(atan);
            __DOUBLE_FUNCTOR_BUILDER(sinh);
            __DOUBLE_FUNCTOR_BUILDER(cosh);
            __DOUBLE_FUNCTOR_BUILDER(tanh);
            __DOUBLE_FUNCTOR_BUILDER(sqrt);

#undef __DOUBLE_FUNCTOR_BUILDER

            struct _sign {
                double operator()(double x) noexcept
                {
                    return x > 0.0F ? 1.0F : (x == 0.0F ? 0.0F : -1.0F);
                }
            };
            fmap["sign"] = math_functor<double, _sign>;

            struct _min {
                double operator()(double x, double y) noexcept
                {
                    return x < y ? x : y;
                }
            };
            fmap["min"] = math_functor2<double, _min>;

            struct _max {
                double operator()(double x, double y) noexcept
                {
                    return x > y ? x : y;
                }
            };
            fmap["max"] = math_functor2<double, _max>;

            struct _pow {
                double operator()(double x, double y) noexcept
                {
                    return ::pow(x, y);
                }
            };
            fmap["pow"] = math_functor2<double, _pow>;

            struct _int_truncate {
                int operator()(double x) noexcept
                {
                    return int((x < 0.0F)
                            ? int((abs(x)) - int(abs(x)) > 0.0F ? int(x) - 1
                                                                : int(x))
                            : int(x));
                }
            };
            fmap["int"] = functor_int_double<_int_truncate>;

            fmap["sqr"] = math_functor<double, _sqrt>;

            struct _rnd {
                double operator()(double x) noexcept
                {
                    if (x < 0.0F) {
                        ::srand((unsigned)time(NULL));
                        ::rand();
                    }

                    return double(::rand()) / RAND_MAX;
                }
            };
            fmap["rnd"] = math_functor<double, _rnd>;

            struct _boolean_not {
                int operator()(double x) noexcept
                {
                    return int((x == 0.0F) ? 1 : 0);
                }
            };
            fmap["not"] = functor_int_double<_boolean_not>;

            struct _bitwise_not {
                int operator()(int x) noexcept { return ~x; }
            };
            fmap["b_not"] = math_functor<int, _bitwise_not>;
        }
    };

} // namespace

const builtin_module_t& get_math_builtin_module()
{
    static math_builtin_module_t module;
    return module;
}

} // namespace nu
