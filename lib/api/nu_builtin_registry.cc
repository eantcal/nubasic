//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_registry.h"

namespace nu {

namespace {

    const std::vector<const builtin_module_t*>& builtin_modules()
    {
        static const std::vector<const builtin_module_t*> modules
            = { &get_math_builtin_module(), &get_string_builtin_module(),
                  &get_runtime_builtin_module(), &get_file_builtin_module(),
                  &get_system_builtin_module(), &get_graphics_builtin_module(),
                  &get_hash_builtin_module() };

        return modules;
    }

} // namespace

const std::vector<const builtin_module_t*>& get_builtin_modules()
{
    return builtin_modules();
}

const builtin_module_t* find_builtin_module(const std::string& name) noexcept
{
    for (const auto* module : builtin_modules()) {
        if (module && module->name() == name) {
            return module;
        }
    }

    return nullptr;
}

void register_builtin_modules(global_function_tbl_t& fmap)
{
    for (const auto* module : builtin_modules()) {
        if (!module) {
            continue;
        }

        module->register_functions(fmap);

        for (const auto& export_name : module->exports()) {
            if (!fmap.is_defined(export_name)) {
                continue;
            }

            const std::string qualified_name
                = module->name() + "::" + export_name;

            if (!fmap.is_defined(qualified_name)) {
                fmap.define(qualified_name, fmap[export_name]);
            }
        }
    }
}

} // namespace nu
