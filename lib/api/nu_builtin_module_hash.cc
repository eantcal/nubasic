//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_registry.h"
#include "nu_hxxx_api.h"

namespace nu {
namespace {

    class hash_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "hash";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports
                = { "hset", "hdel", "hget", "hchk", "hcnt" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            fmap["hset"] = nu::hash_set;
            fmap["hdel"] = nu::hash_del;
            fmap["hget"] = nu::hash_get;
            fmap["hchk"] = nu::hash_chk;
            fmap["hcnt"] = nu::hash_cnt;
        }
    };

} // namespace

const builtin_module_t& get_hash_builtin_module()
{
    static hash_builtin_module_t module;
    return module;
}

} // namespace nu
