//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_native_decl.h"

#include <algorithm>
#include <cctype>

namespace nu {

namespace {

    std::string normalize_native_name(std::string_view name)
    {
        std::string normalized(name);
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
            [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        return normalized;
    }

} // namespace

std::optional<native_type_t> native_type_from_name(std::string_view name)
{
    const auto normalized = normalize_native_name(name);

    if (normalized == "void")
        return native_type_t::Void;
    if (normalized == "integer" || normalized == "int32")
        return native_type_t::Int32;
    if (normalized == "dword" || normalized == "uint32")
        return native_type_t::UInt32;
    if (normalized == "long64" || normalized == "int64")
        return native_type_t::Int64;
    if (normalized == "ulong64" || normalized == "uint64")
        return native_type_t::UInt64;
    if (normalized == "double")
        return native_type_t::Double;
    if (normalized == "bool" || normalized == "boolean")
        return native_type_t::Bool;
    if (normalized == "pointer" || normalized == "ptr")
        return native_type_t::Pointer;
    if (normalized == "string" || normalized == "cstring")
        return native_type_t::CString;

    return std::nullopt;
}

const char* native_type_to_string(native_type_t type) noexcept
{
    switch (type) {
    case native_type_t::Void:
        return "Void";
    case native_type_t::Int32:
        return "Integer";
    case native_type_t::UInt32:
        return "DWORD";
    case native_type_t::Int64:
        return "Long64";
    case native_type_t::UInt64:
        return "ULong64";
    case native_type_t::Double:
        return "Double";
    case native_type_t::Bool:
        return "Bool";
    case native_type_t::Pointer:
        return "Pointer";
    case native_type_t::CString:
        return "String";
    }

    return "Unknown";
}

std::optional<native_calling_convention_t> native_calling_convention_from_name(
    std::string_view name)
{
    const auto normalized = normalize_native_name(name);

    if (normalized == "default")
        return native_calling_convention_t::Default;
    if (normalized == "cdecl")
        return native_calling_convention_t::Cdecl;
    if (normalized == "stdcall")
        return native_calling_convention_t::Stdcall;

    return std::nullopt;
}

const char* native_calling_convention_to_string(
    native_calling_convention_t calling_convention) noexcept
{
    switch (calling_convention) {
    case native_calling_convention_t::Default:
        return "Default";
    case native_calling_convention_t::Cdecl:
        return "Cdecl";
    case native_calling_convention_t::Stdcall:
        return "Stdcall";
    }

    return "Unknown";
}

bool native_function_registry_t::declare(
    native_function_decl_t declaration, std::string* error)
{
    const auto key = normalize_native_name(declaration.basic_name);

    if (_data.find(key) != _data.end()) {
        if (error) {
            *error = "Native function '" + declaration.basic_name
                + "' is already declared";
        }

        return false;
    }

    _data.emplace(key, std::move(declaration));
    return true;
}

const native_function_decl_t* native_function_registry_t::find(
    std::string_view name) const noexcept
{
    const auto it = _data.find(normalize_native_name(name));
    return it == _data.end() ? nullptr : &it->second;
}

bool native_function_registry_t::contains(std::string_view name) const noexcept
{
    return find(name) != nullptr;
}

void native_function_registry_t::clear() noexcept { _data.clear(); }

} // namespace nu
