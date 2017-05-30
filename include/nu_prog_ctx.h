//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_PROG_CTX_H__
#define __NU_PROG_CTX_H__


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
#include <memory>
#include <set>
#include <sstream>
#include <stack>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_ctx_t {
public:
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

    // Procedure metadata (created compiling BASIC source code)
    instrblock_metadata_t procedure_metadata;

    // Struct metadata (created compiling BASIC source code)
    instrblock_metadata_t struct_metadata;
    std::string compiling_struct_name;

    variant_t* get_struct_member_value(
        const std::string& qualified_variable_name,
        var_scope_t::handle_t& scope, size_t index = 0);

    variant_t resolve_struct_element(const std::string& variable_name,
        size_t variable_vect_index, const std::string& element_name,
        size_t element_vect_index, std::string& err_msg);

    prog_ctx_t(FILE* stdout_ptr, FILE* stdin_ptr);

    void clear_metadata();

    // Print-out traces of control-structures and procedure metadata
    void trace_metadata(std::stringstream& ss);

    // Get stdout file pointer
    FILE* get_stdout_ptr() const noexcept { return _stdout_ptr; }

    // Get stdin file pointer
    FILE* get_stdin_ptr() const noexcept { return _stdin_ptr; }

private:
    prog_pointer_t::stmt_number_t _stmt_id_cnt = 0;

    // STD I/O file pointers
    FILE* _stdout_ptr = stdout;
    FILE* _stdin_ptr = stdin;
};


/* -------------------------------------------------------------------------- */
}


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

#endif // __NU_PROG_CTX_H__
