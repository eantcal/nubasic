//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_prog_ctx.h"
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
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::clear_metadata()
{
    // Remove all compile-time control-structures metadata

    for_loop_metadata.clear();
    while_metadata.clear();
    do_loop_while_metadata.clear();
    if_metadata.clear();
    procedure_metadata.clear();
    proc_prototypes.data.clear();
    struct_prototypes.data.clear();
    struct_metadata.clear();
    prog_label.clear();
    compiletime_pc.reset();
    compiling_struct_name.clear();
    stop_metadata.pc_stop_stmt.clear();

    // Remove user-defined-functions
    auto& funcs = global_function_tbl_t::get_instance();

    for (auto& f : function_tbl) {
        funcs.erase(f);
    }

    function_tbl.clear();

    _stmt_id_cnt = 0;
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
    auto scope = proc_scope.get(proc_scope.get_type(variable_name));

    if (!scope || !scope->is_defined(variable_name)) {
        err_msg = "Variable '" + variable_name + "' undefined";
        return variant_t();
    }

    variant_t value = (*scope)[variable_name].first;

    if (!value.is_struct()) {
        err_msg = "Variable '" + variable_name + "' is not a Struct";
        return variant_t();
    }

    auto member_handle = value.struct_member(element_name, variable_vect_index);

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
    ss << "stop-statement metadata:\n" << stop_metadata;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */
