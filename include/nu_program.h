//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_PROGRAM_H__
#define __NU_PROGRAM_H__


/* -------------------------------------------------------------------------- */

#include "nu_flag_map.h"
#include "nu_runnable.h"
#include "nu_stmt.h"

#include <cstdio>
#include <map>
#include <string.h>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct dbginfo_t {
    bool break_point = false;
    bool continue_after_break = false;
    bool single_step_break_point = false;
    std::string condition_str;
    stmt_t::handle_t condition_stmt = nullptr;
};


/* -------------------------------------------------------------------------- */

class prog_line_t : public std::map<runnable_t::line_num_t,
                        std::pair<stmt_t::handle_t, dbginfo_t>> {
};


/* -------------------------------------------------------------------------- */

class prog_line_iterator_t;
class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

class program_t : public runnable_t {
public:
    program_t(prog_line_t& pl, rt_prog_ctx_t& ctx, bool chkpt);


    virtual ~program_t() {}

    bool run(line_num_t start_from) override;


    bool cont(line_num_t start_from, stmt_num_t stmtid) override;


    bool get_dbg_info(line_num_t line, dbginfo_t& dbg);


    bool set_dbg_info(line_num_t line, const dbginfo_t& dbg);


    bool run_next(line_num_t start_from);


    bool run(
        const std::string& name, const std::vector<expr_any_t::handle_t>& args);


    bool run_statement(stmt_t::handle_t stmt_handle, size_t stmt_id,
        prog_line_iterator_t& prog_ptr);


protected:
    bool _run(line_num_t start_from, int stmt_id, bool next);


    void goto_end_block(prog_line_iterator_t& prog_ptr, stmt_t::stmt_cl_t begin,
        stmt_t::stmt_cl_t end, bool& flg);


    rt_prog_ctx_t& get_rt_ctx() noexcept;

private:
    prog_line_t& _prog_line;
    rt_prog_ctx_t& _ctx;
    bool _function_call = false;


    struct checkpoint_data_t {
        flag_map_t flag;
        prog_pointer_t runtime_pc;
        prog_pointer_t goingto_pc;
        return_stack_t return_stack;
        variant_t return_val;
    };
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_PROGRAM_H__
