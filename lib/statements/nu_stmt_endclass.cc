//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_endclass.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_endclass_t::stmt_endclass_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "End Class must be a first line-statement");

    auto handle = ctx.class_metadata.end_find(ctx.compiletime_pc);

    if (!handle) {
        handle = ctx.class_metadata.compile_end(ctx.compiletime_pc);
    }

    if (handle) {
        handle->pc_end_stmt = ctx.compiletime_pc;
    }

    // Inherit data members and visibility from the base class.
    // At this point ctx.compiling_class_name is still set (parse_end clears
    // it only after constructing this statement).
    const std::string& derived = ctx.compiling_class_name;
    if (derived.empty())
        return;

    auto base_it = ctx.class_bases.find(derived);
    if (base_it == ctx.class_bases.end())
        return; // no base class

    const std::string& base_name = base_it->second;
    auto proto_it = ctx.struct_prototypes.data.find(base_name);
    if (proto_it == ctx.struct_prototypes.data.end())
        return;

    auto& derived_proto = ctx.struct_prototypes.data.at(derived).second;
    const variant_t& base_proto = proto_it->second.second;

    // Copy base fields into derived, skipping any already declared there.
    for (const auto& [field_name, field_handle] : base_proto.struct_fields()) {
        if (!field_handle)
            continue;
        // Only add if the derived class did not already declare this field.
        if (derived_proto.struct_fields().count(field_name) == 0) {
            derived_proto.define_struct_member(field_name, *field_handle);
        }
    }

    // Copy base visibility entries into derived (no-overwrite, same principle).
    const std::string base_prefix = base_name + ".";
    std::vector<std::pair<std::string, bool>> to_add;
    for (const auto& [vis_key, is_public] : ctx.class_member_visibility) {
        if (vis_key.rfind(base_prefix, 0) == 0) {
            const std::string member = vis_key.substr(base_prefix.size());
            to_add.emplace_back(derived + "." + member, is_public);
        }
    }
    for (auto& entry : to_add) {
        ctx.class_member_visibility.emplace(
            std::move(entry.first), entry.second);
    }
}


/* -------------------------------------------------------------------------- */

void stmt_endclass_t::run(rt_prog_ctx_t& ctx)
{
    auto handle = ctx.class_metadata.end_find(ctx.runtime_pc);

    if (!handle) {
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::value_t::E_NO_MATCH_CLASS, "");
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_endclass_t::get_cl() const noexcept
{
    return stmt_cl_t::CLASS_END;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
