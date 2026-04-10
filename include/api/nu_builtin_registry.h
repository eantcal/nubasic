//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __NU_BUILTIN_REGISTRY_H__
#define __NU_BUILTIN_REGISTRY_H__

#include "nu_global_function_tbl.h"

#include <string>
#include <vector>

namespace nu {

using builtin_export_list_t = std::vector<std::string>;

class builtin_module_t {
public:
    virtual ~builtin_module_t() = default;
    virtual const std::string& name() const noexcept = 0;
    virtual const builtin_export_list_t& exports() const noexcept = 0;
    virtual void register_functions(global_function_tbl_t& fmap) const = 0;
};

const std::vector<const builtin_module_t*>& get_builtin_modules();
const builtin_module_t* find_builtin_module(const std::string& name) noexcept;

void register_builtin_modules(global_function_tbl_t& fmap);

const builtin_module_t& get_math_builtin_module();
const builtin_module_t& get_string_builtin_module();
const builtin_module_t& get_runtime_builtin_module();
const builtin_module_t& get_file_builtin_module();
const builtin_module_t& get_system_builtin_module();
const builtin_module_t& get_graphics_builtin_module();
const builtin_module_t& get_hash_builtin_module();

} // namespace nu

#endif // __NU_BUILTIN_REGISTRY_H__
