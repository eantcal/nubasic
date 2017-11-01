//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_program.h"
#include "nu_interpreter.h"
#include "nu_rt_prog_ctx.h"
#include "nu_stmt_block.h"
#include "nu_stmt_call.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string.h>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_line_iterator_t : public prog_line_t::iterator {
};


/* -------------------------------------------------------------------------- */

program_t::program_t(prog_line_t& pl, rt_prog_ctx_t& ctx, bool chkpt)
    : _prog_line(pl)
    , _ctx(ctx)
    , _function_call(chkpt)
{
}


/* -------------------------------------------------------------------------- */

void program_t::goto_end_block(prog_line_iterator_t& prog_ptr,
    stmt_t::stmt_cl_t begin, stmt_t::stmt_cl_t end, bool& flg)
{
    int while_cnt = 1;

    while (prog_ptr != _prog_line.end()) {
        if (prog_ptr->second.first->get_cl() == begin) {
            ++while_cnt;
        }
        else if (prog_ptr->second.first->get_cl() == end) {
            --while_cnt;

            if (while_cnt < 1) {
                flg = false;
                break;
            }
        }

        ++prog_ptr;
    }
}


/* -------------------------------------------------------------------------- */

bool program_t::run_statement(stmt_t::handle_t stmt_handle, size_t stmt_id,
    prog_line_iterator_t& prog_ptr)
{
    auto block_ptr = dynamic_cast<stmt_block_t*>(stmt_handle.get());

    if (block_ptr && stmt_id > 0) {
        auto stmt_pos = block_ptr->find_stmt_pos(int(stmt_id));

        if (stmt_pos >= 0) {
            if (++stmt_pos < int(block_ptr->size())) {

                // resume execution from next statement
                if (!block_ptr->run_pos(_ctx, stmt_pos)) {
                    // if not break condition returned
                    // go to next line
                    ++prog_ptr;
                }
            }
            else {
                // no stmt found in this line
                // go to next line
                ++prog_ptr;
            }
        }
        else {
            // no stmt found in this line
            // goto next line
            ++prog_ptr;
        }
    }
    else {
        // stmt executed, for default go to next line
        stmt_handle->run(_ctx);
        ++prog_ptr;
    }

    NU_TRACE_CTX(_ctx);

    return true;
}


/* -------------------------------------------------------------------------- */

bool program_t::run(line_num_t start_from)
{
    return _run(start_from, 0, false);
}


/* -------------------------------------------------------------------------- */

bool program_t::cont(line_num_t start_from, stmt_num_t stmt_id)
{
    return _run(start_from, stmt_id, false);
}


/* -------------------------------------------------------------------------- */

bool program_t::run_next(line_num_t start_from)
{
    return _run(start_from, 0, true);
}


/* -------------------------------------------------------------------------- */

bool program_t::get_dbg_info(line_num_t line, dbginfo_t& dbg)
{
    auto code_line = _prog_line.find(line);

    if (code_line == _prog_line.end()) {
        return false;
    }

    dbg = code_line->second.second;

    return true;
}


/* -------------------------------------------------------------------------- */

bool program_t::set_dbg_info(line_num_t line, const dbginfo_t& dbg)
{
    auto code_line = _prog_line.find(line);

    if (code_line == _prog_line.end()) {
        return false;
    }

    code_line->second.second = dbg;

    return true;
}


/* -------------------------------------------------------------------------- */

bool program_t::_run(line_num_t start_from, stmt_num_t stmt_id, bool next)
{
    checkpoint_data_t cp_data;
    reset_break_event();

    if (_function_call) {
        cp_data.flag = _ctx.flag;
        cp_data.goingto_pc = _ctx.goingto_pc;
        cp_data.runtime_pc = _ctx.runtime_pc;
        cp_data.return_stack = _ctx.return_stack;
        _ctx.return_stack.clear();
    }

    auto prog_ptr = _prog_line.begin();

    if (start_from) {
        auto jump = _prog_line.find(start_from);

        if (next && jump != _prog_line.end()) {
            ++jump;
        }

        if (jump == _prog_line.end()) {
            std::string err = "Line " + nu::to_string(start_from);

            err += next ? " is last line in program" : " does not exist";

            throw exception_t(err);
        }

        prog_ptr = jump;
    }

    auto check_break_event = [&]() {
        if (break_event()) {
            reset_break_event();
            _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
        }
    };

    auto return_stmt_id = stmt_id;

    while (prog_ptr != _prog_line.end()) {
        _ctx.runtime_pc.set(prog_ptr->first, return_stmt_id);

        if (prog_ptr->second.second.single_step_break_point
            && !_function_call) 
        {
            prog_ptr->second.second.single_step_break_point = false;
            _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
            break;
        }

        if (prog_ptr->second.second.break_point) {
            if (prog_ptr->second.second.condition_stmt != nullptr) {
                prog_ptr->second.second.condition_stmt->run(_ctx);
            }
            else {
                _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
            }

            if (_ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]) {
                break;
            }
        }
        else if (prog_ptr->second.second.continue_after_break) {
            // Reset breakpoint for next stmt execution
            prog_ptr->second.second.break_point = true;
            prog_ptr->second.second.continue_after_break = false;
        }


        // Skip execute empty stmt
        if (prog_ptr->second.first->get_cl() == stmt_t::stmt_cl_t::EMPTY) {
            ++prog_ptr;
            continue;
        }

        // Run statement and update prog_ptr
        if (run_statement(prog_ptr->second.first, return_stmt_id,
                static_cast<prog_line_iterator_t&>(prog_ptr))) 
        {
            return_stmt_id = 0;
        }

        check_break_event();
        _yield_host_os();

        if (_ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]) {
            break;
        }

        if (_ctx.flag[rt_prog_ctx_t::FLG_RETURN_REQUEST]) {
            auto return_point = _ctx.get_return_line();
            line_num_t line = return_point.first;
            return_stmt_id = return_point.second;

            _ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, false);

            // If checkpoint enabled and line is zero
            // it means we ware executing a function call (in another stack
            // context) and we have to return to the main program
            if (_function_call && line == 0) {
                break;
            }

            auto jump = _prog_line.find(line);

            if (jump == _prog_line.end()) {
                if (line) {
                    std::string err = "Runtime error. ";
                    err += "Cannot return to line ";
                    err += nu::to_string(prog_ptr->first);
                    err += ": line " + nu::to_string(line);
                    err += " not found.";

                    throw exception_t(err);
                } 
                else {
                    std::string err = "Runtime error.";
                    throw exception_t(err);
                }
            }

            if (jump == _prog_line.end()) {
                break;
            }

            prog_ptr = jump;
        }
        else if (_ctx.flag[rt_prog_ctx_t::FLG_JUMP_REQUEST]) {

            line_num_t line = _ctx.goingto_pc.get_line();
            auto jump = _prog_line.find(line);
            return_stmt_id = 0;
            _ctx.go_to_next(); // ack goto instruction

            if (jump == _prog_line.end()) {
                if (line) {
                    std::string err = "Runtime error at line ";
                    err += nu::to_string(prog_ptr->first);
                    err += ": line " + nu::to_string(line);
                    err += " not found.";

                    throw exception_t(err);
                } 
                else {
                    std::string err = "Runtime error.";
                    throw exception_t(err);
                }
            }

            prog_ptr = jump;

        } 
        else if (_ctx.flag[rt_prog_ctx_t::FLG_SKIP_TILL_NEXT]) {

            auto flg = _ctx.flag.get(rt_prog_ctx_t::FLG_SKIP_TILL_NEXT);

            goto_end_block(static_cast<prog_line_iterator_t&>(prog_ptr),
                stmt_t::stmt_cl_t::FOR_BEGIN, stmt_t::stmt_cl_t::FOR_END, flg);

            _ctx.flag.set(rt_prog_ctx_t::FLG_SKIP_TILL_NEXT, flg);
        }

        if (prog_ptr != _prog_line.end() && _ctx.step_mode_active) {
            auto& breakp = prog_ptr->second.second;
            breakp.single_step_break_point = true;
        }
    }


    bool end_flg = _ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST];
    _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, false);

    NU_TRACE_CTX(_ctx);

    if (_function_call) {
        _ctx.flag = cp_data.flag;
        _ctx.goingto_pc = cp_data.goingto_pc;
        _ctx.runtime_pc = cp_data.runtime_pc;
        _ctx.return_stack = cp_data.return_stack;

        // Propagate end-program flag
        _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, end_flg);
    }

    return prog_ptr != _prog_line.end();
}


/* -------------------------------------------------------------------------- */

bool program_t::run(
    const std::string& name, const std::vector<expr_any_t::handle_t>& args)
{

    // Convert args in the stmt_call_t's argument list
    //
    arg_list_t arg_list;

    for (auto& arg : args) {
        variant_t value = arg->eval(_ctx);

        expr_any_t::handle_t hvalue = std::make_shared<expr_literal_t>(value);

        arg_list.push_back(std::make_pair(
            hvalue, '\0' /*0 is a dummy value, unused in this context*/));
    }

    // Create a call-able object
    stmt_call_t call(arg_list, name, _ctx, true);

    // Create a CALL-statement
    call.run(_ctx, 0);

    // Retrieve the function prototype
    auto prototype = _ctx.proc_prototypes.data.find(name);

    syntax_error_if(prototype == _ctx.proc_prototypes.data.end(),
        "Cannot execute function " + name + ", prototype not found");

    // Finally executes the function
    return run(prototype->second.first.get_line());
}


/* -------------------------------------------------------------------------- */

} // namespace nu
