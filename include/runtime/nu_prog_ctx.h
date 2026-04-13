//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */
#pragma once

/* -------------------------------------------------------------------------- */

#include "nu_file_dscrptr_tbl.h"
#include "nu_flag_map.h"
#include "nu_for_loop_rtdata.h"
#include "nu_instrblock_metadata.h"
#include "nu_label_tbl.h"
#include "nu_proc_prototype_tbl.h"
#include "nu_proc_scope.h"
#include "nu_prog_pointer.h"
#include "nu_symbol_map.h"
#include "nu_var_scope.h"

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_ctx_t {
public:
    enum class syntax_mode_t { LEGACY, MODERN };

    prog_ctx_t() = delete;
    prog_ctx_t(const prog_ctx_t&) = delete;
    prog_ctx_t& operator=(const prog_ctx_t&) = delete;

    virtual ~prog_ctx_t() {}

    prog_pointer_t::stmt_number_t make_next_stmt_id() noexcept
    {
        // first valid id is 1
        return ++_stmt_id_cnt;
    }

    // Enable/disable debug tracing
    bool tracing_on = false;

    // Procedure prototypes
    proc_prototype_tbl_t proc_prototypes;
    std::set<std::string> function_tbl;

    // Structure prototypes
    struct_prototype_tbl_t struct_prototypes;

    // Procedure variable space table
    proc_scope_t proc_scope;

    // Program counter updated at compile time
    prog_pointer_t compiletime_pc;

    // Program label table
    label_tbl_t prog_label;

    // FOR-Loop metadata (created compiling BASIC source code)
    instrblock_metadata_t for_loop_metadata;

    // WHILE-Loop metadata (created compiling BASIC source code)
    instrblock_metadata_t while_metadata;

    // DO-LOOP-WHILE metadata (created compiling BASIC source code)
    instrblock_metadata_t do_loop_while_metadata;

    // IF-statement metadata (created compiling source code)
    if_instrblock_metadata_t if_metadata;

    // SELECT CASE metadata (created compiling BASIC source code)
    select_case_instrblock_metadata_t select_case_metadata;

    // Procedure metadata (created compiling BASIC source code)
    instrblock_metadata_t procedure_metadata;

    // Stop instruction metadata
    stop_instr_metadata_t stop_metadata;

    // Struct metadata (created compiling BASIC source code)
    instrblock_metadata_t struct_metadata;
    std::string compiling_struct_name;

    // Class metadata (created compiling BASIC source code)
    instrblock_metadata_t class_metadata;
    std::string compiling_class_name;
    bool compiling_class_member_is_public = true;

    // Class member access visibility: "ClassName.MemberName" -> is_public
    std::map<std::string, bool> class_member_visibility;

    // Class member declaring owner: "RuntimeClass.MemberName" -> "OwnerClass"
    std::map<std::string, std::string> class_member_owner;

    // Known class names. Classes reuse struct storage but have object
    // reference semantics.
    std::set<std::string> class_names;

    // Single-inheritance chain: derived class name -> direct base class name
    std::map<std::string, std::string> class_bases;

    // Methods declared Overridable.  Enforced at compile time: an Overrides
    // method must target an Overridable method in the base hierarchy, and its
    // signature must match exactly (Sub/Function, return type, parameter count,
    // parameter types, and ByRef/ByVal passing convention).
    std::set<std::string> class_overridable_methods;

    // Methods declared Static: callable as ClassName.Method() without instance
    std::set<std::string> class_static_methods;

    // Set to true while compiling a Static method declaration
    bool compiling_class_member_is_static = false;

    // Set to true while compiling an Overridable method declaration.
    // Cleared by parse_procedure after it records the full mangled name.
    bool compiling_method_is_overridable = false;

    // Set to true while compiling an Overrides method declaration.
    // Cleared by parse_procedure after it validates the override target.
    bool compiling_method_is_override = false;

    // Set to true while loading an included file (depth > 0).
    // Used to suppress "main" registration from included files.
    bool in_include_file = false;

    // Tracks which parameter names are declared ByRef in the procedure
    // currently being compiled.  Cleared by define() after each parameter
    // is registered.
    std::set<std::string> compiling_byref_params;

    variant_t* get_struct_member_value(
        const std::string& qualified_variable_name,
        var_scope_t::handle_t& scope, size_t index = 0,
        std::string* err_msg = nullptr);

    variant_t resolve_struct_element(const std::string& variable_name,
        size_t variable_vect_index, const std::string& element_name,
        size_t element_vect_index, std::string& err_msg);

    std::string current_class_scope_name() const;
    bool is_class_member_access_allowed(const std::string& member_key) const;
    bool is_class_type(const std::string& type_name) const;
    bool is_class_assignable(
        const std::string& target_type, const std::string& source_type) const;

    prog_ctx_t(FILE* stdout_ptr, FILE* stdin_ptr);

    void clear_metadata();

    void set_syntax_mode(syntax_mode_t mode);
    syntax_mode_t get_syntax_mode() const noexcept { return _syntax_mode; }

    void import_builtin_module(const std::string& module_name);
    void import_all_builtin_modules();
    void clear_builtin_module_imports();
    bool is_builtin_module_imported(
        const std::string& module_name) const noexcept
    {
        return _imported_builtin_modules.find(module_name)
            != _imported_builtin_modules.end();
    }

    // Pre-register built-in struct prototypes (DateTime, Mouse) so that
    // "Dim x As DateTime" and "Dim m As Mouse" work without a user-written
    // Struct definition.  Called at construction and after clear_metadata().
    void register_builtin_struct_prototypes();

    // Print-out traces of control-structures and procedure metadata
    void trace_metadata(std::stringstream& ss);

    // Get stdout file pointer
    FILE* get_stdout_ptr() const noexcept { return _stdout_ptr; }

    // Get stdin file pointer
    FILE* get_stdin_ptr() const noexcept { return _stdin_ptr; }

private:
    prog_pointer_t::stmt_number_t _stmt_id_cnt = 0;

    syntax_mode_t _syntax_mode = syntax_mode_t::LEGACY;
    std::set<std::string> _imported_builtin_modules;
    std::set<std::string> _imported_builtin_aliases;

    // STD I/O file pointers
    FILE* _stdout_ptr = stdout;
    FILE* _stdin_ptr = stdin;
};


/* -------------------------------------------------------------------------- */
} // namespace nu


/* -------------------------------------------------------------------------- */

#define NU_TRACE_CTX_AUX(__CTX, _DBGINFO)                                      \
    do {                                                                       \
        std::stringstream ss;                                                  \
        (__CTX).trace_rtdata(ss);                                              \
        if (_DBGINFO)                                                          \
            fprintf((__CTX).get_stdout_ptr(), "\nNU_TRACE_CTX\n%s\n",          \
                __FUNCTION__);                                                 \
        fprintf((__CTX).get_stdout_ptr(), "%s\n", ss.str().c_str());           \
    } while (0)


/* -------------------------------------------------------------------------- */

#define NU_TRACE_CTX(__CTX)                                                    \
    do {                                                                       \
        if ((__CTX).tracing_on) {                                              \
            NU_TRACE_CTX_AUX(__CTX, false);                                    \
        }                                                                      \
    } while (0)


/* -------------------------------------------------------------------------- */
