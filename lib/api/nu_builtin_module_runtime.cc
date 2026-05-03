//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_basic_defs.h"
#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

namespace nu {
namespace {

    void check_native_memory_enabled(
        rt_prog_ctx_t& ctx, const std::string& name)
    {
        if (!ctx.native_calls_enabled) {
            throw exception_t("'" + name
                + "': native memory helpers are disabled by "
                  "--disable-native-calls.");
        }
    }

    std::uint8_t* checked_native_ptr(
        integer_t ptr, const std::string& name, const char* arg_name)
    {
        if (ptr == 0) {
            throw exception_t(
                "'" + name + "': " + arg_name + " must not be null");
        }

        if (ptr < 0) {
            throw exception_t("'" + name + "': " + arg_name
                + " is outside the supported pointer range");
        }

        return reinterpret_cast<std::uint8_t*>(
            static_cast<std::uintptr_t>(ptr));
    }

    std::uint8_t* checked_native_address(rt_prog_ctx_t& ctx,
        const std::string& name, const func_args_t& args, int expected_args)
    {
        check_native_memory_enabled(ctx, name);
        check_arg_num(args, expected_args, name);

        const auto base = args[0]->eval(ctx).to_int();
        const auto offset = args[1]->eval(ctx).to_int();

        if (offset < 0) {
            throw exception_t("'" + name + "': offset must not be negative");
        }

        return checked_native_ptr(base, name, "pointer")
            + static_cast<std::size_t>(offset);
    }

    std::size_t checked_native_size(
        integer_t size, const std::string& name, const char* arg_name)
    {
        if (size < 0) {
            throw exception_t(
                "'" + name + "': " + arg_name + " must not be negative");
        }

        return static_cast<std::size_t>(size);
    }

    integer_t pointer_to_integer(void* ptr)
    {
        return static_cast<integer_t>(reinterpret_cast<std::uintptr_t>(ptr));
    }

    std::size_t bounded_cstring_length(const char* text, std::size_t capacity)
    {
        std::size_t len = 0;
        while (len < capacity && text[len] != '\0') {
            ++len;
        }

        return len;
    }

    template <typename T>
    T checked_native_value(
        integer_t value, const std::string& name, const char* type_name)
    {
        if (value < static_cast<integer_t>(std::numeric_limits<T>::min())
            || value > static_cast<integer_t>(std::numeric_limits<T>::max())) {
            throw exception_t("'" + name + "': value is out of range for "
                + std::string(type_name));
        }

        return static_cast<T>(value);
    }

    template <typename T>
    variant_t native_poke_integer(rt_prog_ctx_t& ctx, const std::string& name,
        const func_args_t& args, const char* type_name)
    {
        auto* address = checked_native_address(ctx, name, args, 3);
        const auto value = args[2]->eval(ctx).to_int();
        const T stored = checked_native_value<T>(value, name, type_name);
        std::memcpy(address, &stored, sizeof(stored));
        return variant_t(integer_t(0));
    }

    class runtime_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "runtime";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports = { "eval",
                "conv", "restore", "sizeof@", "sizeof", "nativealloc",
                "nativefree", "nativefill", "nativepokeb", "nativepokei16",
                "nativepokei32", "nativepokei64", "nativepokeptr",
                "nativepokestr", "nativepeekstr$", "nativepeekstr" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            fmap["eval"] = evaluate_expression;
            fmap["__eval_export"] = evaluate_and_export_result;
            fmap[NU_BASIC_OP_INC] = process_operator;
            fmap[NU_BASIC_OP_DEC] = process_operator;
            fmap["conv"] = conv_functor;
            fmap["restore"] = restore_functor;

            auto functor_sizeof_bv
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::UNDEFINED }, vargs);

                      return variant_t(integer_t(vargs[0].vector_size()));
                  };
            fmap["sizeof@"] = functor_sizeof_bv;

            auto functor_sizeof
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::UNDEFINED }, vargs);

                      return variant_t(integer_t(vargs[0].vector_size()));
                  };
            fmap["sizeof"] = functor_sizeof;

            fmap["nativealloc"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      check_native_memory_enabled(ctx, name);
                      check_arg_num(args, 1, name);

                      const auto size = checked_native_size(
                          args[0]->eval(ctx).to_int(), name, "size");

                      auto deleter = [](void* ptr) { std::free(ptr); };
                      std::unique_ptr<void, decltype(deleter)> memory(
                          std::calloc(1, size == 0 ? 1 : size), deleter);

                      if (!memory) {
                          throw exception_t(
                              "'" + name + "': cannot allocate native memory");
                      }

                      return variant_t(pointer_to_integer(memory.release()));
                  };

            fmap["nativefree"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                check_native_memory_enabled(ctx, name);
                check_arg_num(args, 1, name);

                const auto ptr = args[0]->eval(ctx).to_int();
                if (ptr != 0) {
                    std::free(checked_native_ptr(ptr, name, "pointer"));
                }

                return variant_t(integer_t(0));
            };

            fmap["nativefill"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                auto* address = checked_native_address(ctx, name, args, 4);
                const auto size = checked_native_size(
                    args[2]->eval(ctx).to_int(), name, "size");
                const auto value = checked_native_value<unsigned char>(
                    args[3]->eval(ctx).to_int(), name, "byte");

                std::memset(address, value, size);
                return variant_t(integer_t(0));
            };

            fmap["nativepokeb"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      return native_poke_integer<std::uint8_t>(
                          ctx, name, args, "byte");
                  };

            fmap["nativepokei16"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      return native_poke_integer<std::int16_t>(
                          ctx, name, args, "int16");
                  };

            fmap["nativepokei32"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      return native_poke_integer<std::int32_t>(
                          ctx, name, args, "int32");
                  };

            fmap["nativepokei64"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      return native_poke_integer<std::int64_t>(
                          ctx, name, args, "int64");
                  };

            fmap["nativepokeptr"] = [](rt_prog_ctx_t& ctx,
                                        const std::string& name,
                                        const func_args_t& args) {
                auto* address = checked_native_address(ctx, name, args, 3);
                const auto value = args[2]->eval(ctx).to_int();
                if (value < 0) {
                    throw exception_t("'" + name
                        + "': pointer value is outside the supported range");
                }

                const auto stored = static_cast<std::uintptr_t>(value);
                std::memcpy(address, &stored, sizeof(stored));
                return variant_t(integer_t(0));
            };

            fmap["nativepokestr"] = [](rt_prog_ctx_t& ctx,
                                        const std::string& name,
                                        const func_args_t& args) {
                auto* address = checked_native_address(ctx, name, args, 4);
                const auto text = args[2]->eval(ctx).to_str();
                const auto capacity = checked_native_size(
                    args[3]->eval(ctx).to_int(), name, "capacity");

                if (capacity == 0) {
                    return variant_t(integer_t(0));
                }

                const auto bytes = std::min(capacity - 1, text.size());
                std::memcpy(address, text.data(), bytes);
                address[bytes] = '\0';
                return variant_t(integer_t(bytes));
            };

            auto functor_native_peek_str = [](rt_prog_ctx_t& ctx,
                                               const std::string& name,
                                               const func_args_t& args) {
                auto* address = checked_native_address(ctx, name, args, 3);
                const auto capacity = checked_native_size(
                    args[2]->eval(ctx).to_int(), name, "capacity");

                const auto len = bounded_cstring_length(
                    reinterpret_cast<const char*>(address), capacity);

                return variant_t(
                    std::string(reinterpret_cast<const char*>(address), len));
            };

            fmap["nativepeekstr$"] = functor_native_peek_str;
            fmap["nativepeekstr"] = functor_native_peek_str;
        }
    };

} // namespace

const builtin_module_t& get_runtime_builtin_module()
{
    static runtime_builtin_module_t module;
    return module;
}

} // namespace nu
