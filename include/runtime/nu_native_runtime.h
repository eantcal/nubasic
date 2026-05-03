//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#pragma once

#include "nu_expr_any.h"
#include "nu_native_decl.h"
#include "nu_variant.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace nu {

class native_library_t {
public:
    explicit native_library_t(std::wstring path);
    ~native_library_t();

    native_library_t(const native_library_t&) = delete;
    native_library_t& operator=(const native_library_t&) = delete;

    native_library_t(native_library_t&& other) noexcept;
    native_library_t& operator=(native_library_t&& other) noexcept;

    void* resolve_symbol(std::string_view name) const;
    const std::wstring& path() const noexcept { return _path; }

private:
    std::wstring _path;
    void* _module = nullptr;
};

class native_runtime_t {
public:
    void* resolve(const native_function_decl_t& declaration);
    variant_t invoke(rt_prog_ctx_t& ctx,
        const native_function_decl_t& declaration, const func_args_t& args);
    void clear() noexcept;
    size_t loaded_library_count() const noexcept { return _libraries.size(); }

private:
    std::shared_ptr<native_library_t> load_or_get_library(
        const std::wstring& path);

    std::unordered_map<std::wstring, std::shared_ptr<native_library_t>>
        _libraries;
};

} // namespace nu
