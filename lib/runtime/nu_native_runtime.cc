//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_native_runtime.h"
#include "nu_error_codes.h"
#include "nu_exception.h"
#include "nu_rt_prog_ctx.h"

#include <array>
#include <cstdint>
#include <limits>
#include <utility>

#ifdef NUBASIC_HAS_LIBFFI
#include <ffi.h>
#endif

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

namespace nu {

namespace {

    std::string narrow(std::wstring_view text)
    {
        std::string result;
        result.reserve(text.size());

        for (const auto ch : text) {
            result.push_back(static_cast<char>(ch));
        }

        return result;
    }

#ifdef _WIN32
    std::string last_windows_error()
    {
        const DWORD error = GetLastError();

        if (error == 0) {
            return {};
        }

        LPSTR buffer = nullptr;
        const DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

        std::string message;

        if (len != 0 && buffer) {
            message.assign(buffer, len);
            while (!message.empty()
                && (message.back() == '\r' || message.back() == '\n'
                    || message.back() == ' ')) {
                message.pop_back();
            }
        } else {
            message = "Windows error " + std::to_string(error);
        }

        if (buffer) {
            LocalFree(buffer);
        }

        return message;
    }
#endif

#ifdef NUBASIC_HAS_LIBFFI
    ffi_type* to_ffi_type(native_type_t type)
    {
        switch (type) {
        case native_type_t::Void:
            return &ffi_type_void;
        case native_type_t::Int32:
        case native_type_t::Bool:
            return &ffi_type_sint32;
        case native_type_t::UInt32:
            return &ffi_type_uint32;
        case native_type_t::Int64:
            return &ffi_type_sint64;
        case native_type_t::UInt64:
            return &ffi_type_uint64;
        case native_type_t::Pointer:
            return &ffi_type_pointer;
        case native_type_t::Double:
            return &ffi_type_double;
        case native_type_t::CString:
            return &ffi_type_pointer;
        }

        return &ffi_type_void;
    }

    struct native_arg_storage_t {
        int32_t i32 = 0;
        uint32_t u32 = 0;
        int64_t i64 = 0;
        uint64_t u64 = 0;
        double d = 0.0;
        void* ptr = nullptr;
        const char* cstr = nullptr;
        std::string string_value;

        template <typename T>
        static T checked_integral(
            integer_t value, const std::string& name, native_type_t type)
        {
            if (value < static_cast<integer_t>(std::numeric_limits<T>::min())
                || value
                    > static_cast<integer_t>(std::numeric_limits<T>::max())) {
                throw exception_t("Native argument '" + name
                    + "' value is out of range for "
                    + native_type_to_string(type));
            }

            return static_cast<T>(value);
        }

        void* set(
            native_type_t type, const std::string& name, const variant_t& value)
        {
            switch (type) {
            case native_type_t::Int32:
                i32 = checked_integral<int32_t>(value.to_int(), name, type);
                return &i32;
            case native_type_t::UInt32:
                u32 = checked_integral<uint32_t>(value.to_int(), name, type);
                return &u32;
            case native_type_t::Int64:
                i64 = static_cast<int64_t>(value.to_int());
                return &i64;
            case native_type_t::UInt64:
                if (value.to_int() < 0) {
                    throw exception_t("Native argument '" + name
                        + "' value is out of range for "
                        + native_type_to_string(type));
                }
                u64 = static_cast<uint64_t>(value.to_int());
                return &u64;
            case native_type_t::Pointer:
                if (value.to_int() < 0) {
                    throw exception_t("Native argument '" + name
                        + "' value is out of range for "
                        + native_type_to_string(type));
                }
                ptr = reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(value.to_int()));
                return &ptr;
            case native_type_t::Double:
                d = value.to_double();
                return &d;
            case native_type_t::Bool:
                i32 = value.to_bool() ? 1 : 0;
                return &i32;
            case native_type_t::CString:
                string_value = value.to_str();
                cstr = string_value.c_str();
                return &cstr;
            case native_type_t::Void:
                break;
            }

            throw exception_t("Void is not a valid native argument type");
        }
    };

    variant_t native_return_to_variant(
        native_type_t type, const std::array<std::uint8_t, 16>& storage)
    {
        switch (type) {
        case native_type_t::Void:
            return variant_t(integer_t(0));
        case native_type_t::Int32:
            return variant_t(
                integer_t(*reinterpret_cast<const int32_t*>(storage.data())));
        case native_type_t::UInt32:
            return variant_t(
                integer_t(*reinterpret_cast<const uint32_t*>(storage.data())));
        case native_type_t::Int64:
            return variant_t(
                integer_t(*reinterpret_cast<const int64_t*>(storage.data())));
        case native_type_t::UInt64:
        case native_type_t::Pointer:
            return variant_t(
                integer_t(*reinterpret_cast<const uint64_t*>(storage.data())));
        case native_type_t::Double:
            return variant_t(*reinterpret_cast<const double*>(storage.data()));
        case native_type_t::Bool:
            return variant_t(
                bool_t(*reinterpret_cast<const int32_t*>(storage.data()) != 0));
        case native_type_t::CString: {
            const auto ptr = *reinterpret_cast<char* const*>(storage.data());
            return variant_t(ptr ? ptr : "");
        }
        }

        return variant_t(integer_t(0));
    }
#endif

} // namespace

native_library_t::native_library_t(std::wstring path)
    : _path(std::move(path))
{
#ifdef _WIN32
    _module = LoadLibraryW(_path.c_str());

    if (!_module) {
        throw exception_t("Cannot load native library '" + narrow(_path)
            + "': " + last_windows_error());
    }
#elif defined(__unix__) || defined(__APPLE__)
    const std::string narrow_path = narrow(_path);
    _module = dlopen(narrow_path.c_str(), RTLD_LAZY | RTLD_LOCAL);

    if (!_module) {
        const char* err = dlerror();
        throw exception_t("Cannot load native library '" + narrow_path
            + "': " + (err ? err : "unknown error"));
    }
#else
    (void)_path;
    throw exception_t("Native DLL calls are not supported on this platform.");
#endif
}

native_library_t::~native_library_t()
{
#ifdef _WIN32
    if (_module) {
        FreeLibrary(static_cast<HMODULE>(_module));
    }
#elif defined(__unix__) || defined(__APPLE__)
    if (_module) {
        dlclose(_module);
    }
#endif
}

native_library_t::native_library_t(native_library_t&& other) noexcept
    : _path(std::move(other._path))
    , _module(other._module)
{
    other._module = nullptr;
}

native_library_t& native_library_t::operator=(native_library_t&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

#ifdef _WIN32
    if (_module) {
        FreeLibrary(static_cast<HMODULE>(_module));
    }
#elif defined(__unix__) || defined(__APPLE__)
    if (_module) {
        dlclose(_module);
    }
#endif

    _path = std::move(other._path);
    _module = other._module;
    other._module = nullptr;

    return *this;
}

void* native_library_t::resolve_symbol(std::string_view name) const
{
#ifdef _WIN32
    const std::string symbol(name);
    void* proc = reinterpret_cast<void*>(
        GetProcAddress(static_cast<HMODULE>(_module), symbol.c_str()));

    if (!proc) {
        throw exception_t("Cannot resolve native symbol '" + symbol + "' from '"
            + narrow(_path) + "': " + last_windows_error());
    }

    return proc;
#elif defined(__unix__) || defined(__APPLE__)
    const std::string symbol(name);
    // Clear any pending error so we can distinguish "symbol resolved to NULL"
    // from "symbol not found" by checking dlerror() right after dlsym().
    dlerror();
    void* proc = dlsym(_module, symbol.c_str());
    const char* err = dlerror();

    if (err) {
        throw exception_t("Cannot resolve native symbol '" + symbol + "' from '"
            + narrow(_path) + "': " + err);
    }

    return proc;
#else
    (void)name;
    throw exception_t("Native DLL calls are not supported on this platform.");
#endif
}

void* native_runtime_t::resolve(const native_function_decl_t& declaration)
{
    auto library = load_or_get_library(declaration.library_name);
    return library->resolve_symbol(declaration.export_name);
}

variant_t native_runtime_t::invoke(rt_prog_ctx_t& ctx,
    const native_function_decl_t& declaration, const func_args_t& args)
{
    if (!ctx.native_calls_enabled) {
        throw exception_t(
            "Native DLL calls are disabled by --disable-native-calls.");
    }

#ifndef NUBASIC_HAS_LIBFFI
    (void)ctx;
    (void)declaration;
    (void)args;
    throw exception_t(
        "Native DLL invocation is not available because libffi was not found "
        "at build time.");
#else
    const auto line = ctx.runtime_pc.get_line();
    auto& errors = rt_error_code_t::get_instance();

    errors.throw_if(args.size() != declaration.parameters.size(), line,
        rt_error_code_t::value_t::E_WRG_NUM_ARGS, declaration.basic_name);

    std::vector<ffi_type*> arg_types;
    std::vector<native_arg_storage_t> arg_storage(args.size());
    std::vector<void*> arg_values;

    arg_types.reserve(args.size());
    arg_values.reserve(args.size());

    for (size_t idx = 0; idx < args.size(); ++idx) {
        const auto target_type = declaration.parameters[idx].type;
        arg_types.push_back(to_ffi_type(target_type));

        const auto value = args[idx]->eval(ctx);
        arg_values.push_back(arg_storage[idx].set(
            target_type, declaration.parameters[idx].name, value));
    }

    ffi_cif cif;
    ffi_type* return_type = to_ffi_type(declaration.return_type);

    const ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
        static_cast<unsigned int>(arg_types.size()), return_type,
        arg_types.empty() ? nullptr : arg_types.data());

    if (status != FFI_OK) {
        throw exception_t("Failed to prepare native call interface for '"
            + declaration.basic_name + "'");
    }

    std::array<std::uint8_t, 16> return_storage{};
    void* function = resolve(declaration);

    ffi_call(&cif, FFI_FN(function),
        declaration.return_type == native_type_t::Void ? nullptr
                                                       : return_storage.data(),
        arg_values.empty() ? nullptr : arg_values.data());

    return native_return_to_variant(declaration.return_type, return_storage);
#endif
}

void native_runtime_t::clear() noexcept { _libraries.clear(); }

std::shared_ptr<native_library_t> native_runtime_t::load_or_get_library(
    const std::wstring& path)
{
    auto it = _libraries.find(path);

    if (it != _libraries.end()) {
        return it->second;
    }

    auto library = std::make_shared<native_library_t>(path);
    _libraries.emplace(path, library);
    return library;
}

} // namespace nu
