//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_prog_ctx.h"
#include "nu_builtin_registry.h"
#include "nu_global_function_tbl.h"
#include "nu_os_std.h"

#include <iomanip>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

prog_ctx_t::prog_ctx_t(FILE* stdout_ptr, FILE* stdin_ptr)
    : _stdout_ptr(stdout_ptr)
    , _stdin_ptr(stdin_ptr)
{
    register_builtin_struct_prototypes();
    set_syntax_mode(syntax_mode_t::LEGACY);
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::clear_metadata()
{
    // Remove all compile-time control-structures metadata

    for_loop_metadata.clear();
    while_metadata.clear();
    do_loop_while_metadata.clear();
    if_metadata.clear();
    select_case_metadata.clear();
    procedure_metadata.clear();
    proc_prototypes.data.clear();
    struct_prototypes.data.clear();
    struct_metadata.clear();
    class_metadata.clear();
    prog_label.clear();
    compiletime_pc.reset();
    compiling_struct_name.clear();
    compiling_class_name.clear();
    class_member_visibility.clear();
    class_bases.clear();
    class_overridable_methods.clear();
    class_static_methods.clear();
    compiling_class_member_is_static = false;
    in_include_file = false;
    stop_metadata.pc_stop_stmt.clear();

    // Remove user-defined-functions
    auto& funcs = global_function_tbl_t::get_instance();

    for (auto& f : function_tbl) {
        funcs.erase(f);
    }

    function_tbl.clear();

    clear_builtin_module_imports();
    _syntax_mode = syntax_mode_t::LEGACY;
    import_all_builtin_modules();

    _stmt_id_cnt = 0;

    register_builtin_struct_prototypes();
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::clear_builtin_module_imports()
{
    auto& funcs = global_function_tbl_t::get_instance();

    for (const auto& alias_name : _imported_builtin_aliases) {
        if (function_tbl.find(alias_name) == function_tbl.end()) {
            funcs.erase(alias_name);
        }
    }

    for (const auto* module : get_builtin_modules()) {
        if (!module) {
            continue;
        }

        for (const auto& export_name : module->exports()) {
            if (function_tbl.find(export_name) == function_tbl.end()) {
                funcs.erase(export_name);
            }
        }
    }

    _imported_builtin_aliases.clear();
    _imported_builtin_modules.clear();
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::import_builtin_module(const std::string& module_name)
{
    const auto* module = find_builtin_module(module_name);

    if (!module) {
        throw exception_t("Module '" + module_name + "' is not defined");
    }

    auto& funcs = global_function_tbl_t::get_instance();

    for (const auto& export_name : module->exports()) {
        const std::string qualified_name = module_name + "::" + export_name;

        if (!funcs.is_defined(qualified_name)) {
            throw exception_t("Module '" + module_name
                + "' has an incomplete registration for '" + export_name + "'");
        }

        if (funcs.is_defined(export_name)) {
            if (_imported_builtin_aliases.find(export_name)
                != _imported_builtin_aliases.end()) {
                continue;
            }

            throw exception_t("Cannot import module '" + module_name
                + "': symbol '" + export_name + "' is already defined");
        }

        funcs.define(export_name, funcs[qualified_name]);
        _imported_builtin_aliases.insert(export_name);
    }

    _imported_builtin_modules.insert(module_name);
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::import_all_builtin_modules()
{
    for (const auto* module : get_builtin_modules()) {
        if (module) {
            import_builtin_module(module->name());
        }
    }
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::set_syntax_mode(syntax_mode_t mode)
{
    clear_builtin_module_imports();
    _syntax_mode = mode;

    if (mode == syntax_mode_t::LEGACY) {
        import_all_builtin_modules();
    }
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::register_builtin_struct_prototypes()
{
    // DateTime — all date/time fields in one struct.
    {
        variant_t proto("datetime", variant_t::struct_data_t{});
        proto.define_struct_member(
            "year", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "month", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "day", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "hour", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "minute", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "second", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "wday", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "yday", variant_t(string_t(), variable_t::type_t::INTEGER));
        struct_prototypes.data["datetime"]
            = std::make_pair(prog_pointer_t{}, proto);
    }

    // Mouse — pointer position and button state in one struct.
    {
        variant_t proto("mouse", variant_t::struct_data_t{});
        proto.define_struct_member(
            "x", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "y", variant_t(string_t(), variable_t::type_t::INTEGER));
        proto.define_struct_member(
            "btn", variant_t(string_t(), variable_t::type_t::INTEGER));
        struct_prototypes.data["mouse"]
            = std::make_pair(prog_pointer_t{}, proto);
    }
}


/* -------------------------------------------------------------------------- */

static std::vector<std::string> split(const std::string& s, char delim = '.')
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}


/* -------------------------------------------------------------------------- */

variant_t* prog_ctx_t::get_struct_member_value(
    const std::string& qualified_variable_name, var_scope_t::handle_t& scope,
    size_t index)
{
    auto reflist = split(qualified_variable_name);
    const auto& member = reflist[0];

    scope = proc_scope.get(proc_scope.get_type(member));

    variant_t* value = nullptr;

    if (scope->is_defined(member)) {
        value = &((*scope)[member].first);

        size_t level = 1;
        while (value && value->is_struct() && level < reflist.size()) {
            const auto& member_name = reflist[level++];

            auto value_handle = value->struct_member(member_name, index);
            value = value_handle.get();
        }
    }

    return value;
}


/* -------------------------------------------------------------------------- */

variant_t prog_ctx_t::resolve_struct_element(const std::string& variable_name,
    size_t variable_vect_index, const std::string& element_name,
    size_t element_vect_index, std::string& err_msg)
{
    // variable_name may be a multi-level path like "seg.a" when
    // expr_struct_access_t is nested.  Use get_struct_member_value to navigate
    // to the parent, then look up element_name on that result.
    var_scope_t::handle_t scope;
    variant_t* value = nullptr;

    if (variable_name.find('.') != std::string::npos) {
        // Multi-level path (e.g. "seg.a"): navigate to the nested struct first,
        // then look up element_name on its result.
        value = get_struct_member_value(
            variable_name, scope, variable_vect_index);
        if (!value) {
            err_msg = "Variable '" + variable_name + "' undefined";
            return variant_t();
        }
    } else {
        scope = proc_scope.get(proc_scope.get_type(variable_name));

        if (!scope || !scope->is_defined(variable_name)) {
            err_msg = "Variable '" + variable_name + "' undefined";
            return variant_t();
        }

        value = &(*scope)[variable_name].first;
    }

    if (!value->is_struct()) {
        err_msg = "Variable '" + variable_name + "' is not a Struct";
        return variant_t();
    }

    auto member_handle
        = value->struct_member(element_name, variable_vect_index);

    if (!member_handle) {
        err_msg = "Variable '" + variable_name + "' has not member named '"
            + element_name + "'";

        return variant_t();
    }

    if (member_handle->is_vector()) {
        if (element_vect_index >= member_handle->vector_size()) {
            err_msg = "Element vector index of '" + element_name
                + "' is out of range";

            return variant_t();
        }

        return (*member_handle)[element_vect_index];
    }

    return (*member_handle);
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::trace_metadata(std::stringstream& ss)
{
    ss << "Syntax mode : "
       << (_syntax_mode == syntax_mode_t::LEGACY ? "legacy" : "modern")
       << std::endl;

    if (!_imported_builtin_modules.empty()) {
        ss << "Imported modules : ";

        auto it = _imported_builtin_modules.begin();
        while (it != _imported_builtin_modules.end()) {
            ss << *it;
            ++it;

            if (it != _imported_builtin_modules.end()) {
                ss << ", ";
            }
        }

        ss << std::endl;
    }

    ss << "Explicit line number reference detected: "
       << (prog_label.get_explicit_line_reference_mode() ? "Y" : "N")
       << std::endl;

    ss << "Last compiled line : " << compiletime_pc.get_line() << std::endl;
    ss << "Last compiled stmt : " << compiletime_pc.get_stmt_pos() << std::endl;

    if (!proc_prototypes.data.empty()) {
        ss << "Procedures Prototypes:\n";

        for (const auto& s : proc_prototypes.data) {
            ss << "\tline:" << std::setw(5) << s.second.first.get_line() << " ";
            ss << (function_tbl.find(s.first) != function_tbl.end() ? "Func "
                                                                    : "Sub  ");

            ss << s.first << "(";

            const auto& parameters = s.second.second.parameters;

            auto size = parameters.size();

            for (const auto& p : parameters) {
                ss << p.var_name;

                if (!p.type_name.empty())
                    ss << " As " << p.type_name;

                if (--size > 0)
                    ss << ",";
            }

            ss << ")\n";
        }
    }

    if (!struct_prototypes.data.empty()) {
        ss << "Structure Prototypes:\n";

        for (const auto& s : struct_prototypes.data) {
            ss << "\tline:" << std::setw(5) << s.second.first.get_line() << " ";
            ss << "Struct " << s.first << "\n";
            s.second.second.describe_type(ss);
            ss << "\n";
        }
    }

    if (!procedure_metadata.empty()) {
        ss << "Sub Procedures:\n";
        procedure_metadata.trace(ss);
        ss << std::endl;
    }

    if (!struct_metadata.empty()) {
        ss << "Structures:\n";
        struct_metadata.trace(ss);
        ss << std::endl;
    }

    if (!prog_label.empty()) {
        ss << "Labels:\n";
        ss << prog_label;
    }

    if (!while_metadata.empty()) {
        ss << "While loop metadata:\n";
        while_metadata.trace(ss);
        ss << std::endl;
    }

    if (!for_loop_metadata.empty()) {
        ss << "For-loop metadata:\n";
        for_loop_metadata.trace(ss);
        ss << std::endl;
    }

    if (!do_loop_while_metadata.empty()) {
        ss << "Do-Loop-While metadata:\n";
        do_loop_while_metadata.trace(ss);
        ss << std::endl;
    }

    ss << "If-statement metadata:\n" << if_metadata;

    if (!stop_metadata.pc_stop_stmt.empty()) {
        ss << "stop-statement metadata:\n" << stop_metadata;
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
