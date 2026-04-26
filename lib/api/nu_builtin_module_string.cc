//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"
#include "nu_os_console.h"

#include <algorithm>
#include <cstdio>
#include <sstream>

namespace nu {
namespace {

    class string_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "string";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports = { "len$", "len",
                "input$", "asc$", "asc", "spc", "space$", "chr$", "chr", "left",
                "left$", "lcase", "lcase$", "ucase", "ucase$", "right",
                "right$", "instrcs", "instr", "substr$", "mid$", "mid", "pstr$",
                "val", "val%", "str", "str$", "strp", "strp$", "hex$", "hex" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            struct _len_str {
                int operator()(const std::string x) noexcept
                {
                    return int(x.size());
                }
            };
            fmap["len$"] = functor_int_string<_len_str>;
            fmap["len"] = functor_int_string<_len_str>;

            fmap["input$"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                 const func_args_t& args) -> variant_t {
                check_arg_num(args, 1, name);
                const auto value = args[0]->eval(ctx);

                syntax_error_if(value.get_type() != variant_t::type_t::INTEGER,
                    "'" + name
                        + "': expects to be passed argument 1 as Integer");

                auto input_result = _os_input_str_interruptible(
                    static_cast<int>(value.to_int()));
                if (input_result.interrupted) {
                    ctx.flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, true);
                    throw debug_suspend_t();
                }

                return variant_t(input_result.text);
            };

            struct _asc_str {
                int operator()(const std::string x) noexcept
                {
                    return (x.empty() ? 0 : x.c_str()[0]) & 0xff;
                }
            };
            fmap["asc$"] = functor_int_string<_asc_str>;
            fmap["asc"] = functor_int_string<_asc_str>;

            struct _space_str {
                std::string operator()(int nspace) noexcept
                {
                    std::string s;
                    nspace = nspace < 0 ? 0 : nspace;

                    for (int i = 0; i < nspace; ++i) {
                        s += " ";
                    }

                    return s;
                }
            };
            fmap["spc"] = functor_string_int<_space_str>;
            fmap["space$"] = functor_string_int<_space_str>;

            struct _chr_str {
                std::string operator()(int code) noexcept
                {
                    char s[2] = { 0 };
                    s[0] = code;
                    return s;
                }
            };
            fmap["chr$"] = functor_string_int<_chr_str>;
            fmap["chr"] = functor_string_int<_chr_str>;

            struct _left_str {
                std::string operator()(const std::string& s, int n) noexcept
                {
                    if (n <= 0) {
                        return std::string();
                    } else if (n > int(s.size())) {
                        n = int(s.size());
                    }

                    return s.substr(0, n);
                }
            };
            fmap["left"] = functor_string_string_int<_left_str>;
            fmap["left$"] = functor_string_string_int<_left_str>;

            struct _lcase_str {
                std::string operator()(const std::string& s) noexcept
                {
                    std::string ret = s;
                    std::transform(
                        ret.begin(), ret.end(), ret.begin(), ::tolower);
                    return ret;
                }
            };
            fmap["lcase"] = functor_string_string<_lcase_str>;
            fmap["lcase$"] = functor_string_string<_lcase_str>;

            struct _ucase_str {
                std::string operator()(const std::string& s) noexcept
                {
                    std::string ret = s;
                    std::transform(
                        ret.begin(), ret.end(), ret.begin(), ::toupper);
                    return ret;
                }
            };
            fmap["ucase"] = functor_string_string<_ucase_str>;
            fmap["ucase$"] = functor_string_string<_ucase_str>;

            struct _right_str {
                std::string operator()(const std::string& s, int n) noexcept
                {
                    if (n <= 0) {
                        return s;
                    } else if (n > int(s.size())) {
                        n = int(s.size());
                    }

                    const int skip = int(s.size()) - n;
                    return s.substr(skip, n);
                }
            };
            fmap["right"] = functor_string_string_int<_right_str>;
            fmap["right$"] = functor_string_string_int<_right_str>;

            struct _instrcs {
                int operator()(const std::string& s,
                    const std::string& search_str) noexcept
                {
                    if (search_str.empty()) {
                        return 0;
                    } else if (s.empty() || search_str.size() > s.size()) {
                        return -1;
                    } else if (search_str.size() == s.size()
                        && s == search_str) {
                        return 0;
                    }

                    std::string mystr = s;
                    int pos = 0;

                    while (search_str.size() <= mystr.size()) {
                        if (mystr.substr(0, search_str.size()) == search_str) {
                            return pos;
                        }

                        ++pos;
                        mystr = mystr.substr(1, mystr.size() - 1);
                    }

                    return -1;
                }
            };
            fmap["instrcs"] = functor_int_string_string<_instrcs>;

            struct _instr : public _instrcs {
                int operator()(const std::string& s,
                    const std::string& search_str) noexcept
                {
                    std::string mys = s;
                    std::string ss = search_str;
                    std::transform(
                        mys.begin(), mys.end(), mys.begin(), toupper);
                    std::transform(ss.begin(), ss.end(), ss.begin(), toupper);
                    return _instrcs::operator()(mys, ss);
                }
            };
            fmap["instr"] = functor_int_string_string<_instr>;

            struct _sub_str {
                std::string operator()(
                    const std::string& s, int pos, int n) noexcept
                {
                    if (pos < 1) {
                        pos = 0;
                    }

                    if (pos >= int(s.size())) {
                        return std::string();
                    }

                    if (n < 0) {
                        n = 0;
                    }

                    if ((pos + n) >= int(s.size())) {
                        n = int(s.size()) - pos;
                    }

                    return s.substr(pos, n);
                }
            };
            fmap["substr$"] = functor_string_string_int_int<_sub_str>;

            struct _mid_str {
                std::string operator()(
                    const std::string& s, int pos, int n) noexcept
                {
                    --pos;

                    if (pos < 1) {
                        pos = 0;
                    }

                    if (pos >= int(s.size())) {
                        return std::string();
                    }

                    if (n < 0) {
                        n = 0;
                    }

                    if ((pos + n) >= int(s.size())) {
                        n = int(s.size()) - pos;
                    }

                    return s.substr(pos, n);
                }
            };
            fmap["mid$"] = functor_string_string_int_int<_mid_str>;
            fmap["mid"] = functor_string_string_int_int<_mid_str>;

            struct _patch_str {
                std::string operator()(const std::string& s, int pos,
                    const std::string& c_s) noexcept
                {
                    if (pos >= int(s.size())) {
                        pos = int(s.size() - 1);
                    }

                    if (pos < 1) {
                        pos = 0;
                    }

                    std::string ret = s;
                    const char c = c_s.empty() ? '\0' : c_s[0];
                    ret[pos] = c;
                    return ret;
                }
            };
            fmap["pstr$"] = functor_string_string_int_string<_patch_str>;

            struct _val_str {
                double operator()(const std::string& x) noexcept
                {
                    try {
                        return nu::stod(x);
                    } catch (...) {
                        return 0.0;
                    }
                }
            };
            fmap["val"] = functor_double_string<_val_str>;
            fmap["val%"] = functor_int_string<_val_str>;

            struct _to_str {
                std::string operator()(double x) noexcept
                {
                    if (::floor(x) == x) {
                        return to_string(int(x));
                    }

                    return to_string(x);
                }
            };
            fmap["str"] = functor_string_double<_to_str>;
            fmap["str$"] = functor_string_double<_to_str>;

            struct _to_str_precision {
                std::string operator()(double x, int digits) noexcept
                {
                    char buffer[1024] = { 0 };
                    const std::string format
                        = "%." + std::to_string(abs(digits)) + "f";
                    snprintf(buffer, sizeof(buffer) - 1, format.c_str(), x);
                    return buffer;
                }
            };
            fmap["strp"] = functor_string_double_int<_to_str_precision>;
            fmap["strp$"] = functor_string_double_int<_to_str_precision>;

            struct _to_hex_str {
                std::string operator()(double x) noexcept
                {
                    std::stringstream ss;
                    ss << std::hex << int(x);
                    return ss.str();
                }
            };
            fmap["hex$"] = functor_string_double<_to_hex_str>;
            fmap["hex"] = functor_string_double<_to_hex_str>;
        }
    };

} // namespace

const builtin_module_t& get_string_builtin_module()
{
    static string_builtin_module_t module;
    return module;
}

} // namespace nu
