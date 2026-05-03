//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace nu {

enum class native_type_t {
    Void,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Double,
    Bool,
    Pointer,
    CString
};

enum class native_calling_convention_t { Default, Cdecl, Stdcall };

struct native_parameter_t {
    std::string name;
    native_type_t type = native_type_t::Void;
};

struct native_function_decl_t {
    std::string basic_name;
    std::wstring library_name;
    std::string export_name;
    native_calling_convention_t calling_convention
        = native_calling_convention_t::Default;
    native_type_t return_type = native_type_t::Void;
    std::vector<native_parameter_t> parameters;
};

std::optional<native_type_t> native_type_from_name(std::string_view name);
const char* native_type_to_string(native_type_t type) noexcept;

std::optional<native_calling_convention_t> native_calling_convention_from_name(
    std::string_view name);
const char* native_calling_convention_to_string(
    native_calling_convention_t calling_convention) noexcept;

class native_function_registry_t {
public:
    bool declare(
        native_function_decl_t declaration, std::string* error = nullptr);
    const native_function_decl_t* find(std::string_view name) const noexcept;
    bool contains(std::string_view name) const noexcept;
    void clear() noexcept;
    size_t size() const noexcept { return _data.size(); }

private:
    std::unordered_map<std::string, native_function_decl_t> _data;
};

} // namespace nu
