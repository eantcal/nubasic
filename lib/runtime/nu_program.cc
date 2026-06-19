//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_program.h"
#include "nu_expr_literal.h"
#include "nu_interpreter.h"
#include "nu_rt_prog_ctx.h"
#include "nu_stmt_block.h"
#include "nu_stmt_call.h"
#include "nu_var_scope.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <memory>
#include <string.h>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_line_iterator_t : public prog_line_t::iterator {};


/* -------------------------------------------------------------------------- */

static std::string procedure_name_from_scope(const std::string& scope_id)
{
    const auto bracket = scope_id.find('[');
    return bracket == std::string::npos ? scope_id
                                        : scope_id.substr(0, bracket);
}


/* -------------------------------------------------------------------------- */

static bool source_line_has_blocking_input(std::string line)
{
    bool in_string = false;
    std::string code;
    code.reserve(line.size());

    for (const auto ch : line) {
        if (ch == '"') {
            in_string = !in_string;
        } else if (ch == '\'' && !in_string) {
            break;
        }

        code += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    const auto skip_spaces = [&](size_t pos) {
        while (pos < code.size()
            && std::isspace(static_cast<unsigned char>(code[pos]))) {
            ++pos;
        }
        return pos;
    };

    size_t pos = skip_spaces(0);
    while (pos < code.size()
        && std::isdigit(static_cast<unsigned char>(code[pos])))
        ++pos;
    pos = skip_spaces(pos);

    if (code.compare(pos, 5, "input") == 0) {
        const auto next = pos + 5;
        return next >= code.size()
            || std::isspace(static_cast<unsigned char>(code[next]))
            || code[next] == '$';
    }

    return code.find("input$") != std::string::npos;
}


/* -------------------------------------------------------------------------- */

program_t::program_t(prog_line_t& pl, rt_prog_ctx_t& ctx, bool chkpt)
    : _prog_line(pl)
    , _ctx(ctx)
    , _function_call(chkpt)
    , _debug_expression_call(chkpt)
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
        } else if (prog_ptr->second.first->get_cl() == end) {
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
            } else {
                // no stmt found in this line
                // go to next line
                ++prog_ptr;
            }
        } else {
            // no stmt found in this line
            // goto next line
            ++prog_ptr;
        }
    } else {
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

    const bool debug_function_call
        = _function_call && _debug_expression_call && _ctx.debug_mode;
    const bool using_debug_function_checkpoint
        = debug_function_call && !_ctx.debug_function_checkpoints.empty();
    std::string function_name;
    if (debug_function_call) {
        function_name
            = procedure_name_from_scope(_ctx.proc_scope.get_scope_id());
    }
    line_num_t debug_function_call_site_line = 0;

    if (_function_call) {
        if (using_debug_function_checkpoint) {
            const auto& checkpoint = _ctx.debug_function_checkpoints.front();
            cp_data.flag = checkpoint.caller_flag;
            cp_data.goingto_pc = checkpoint.caller_goingto_pc;
            cp_data.runtime_pc = checkpoint.caller_runtime_pc;
            cp_data.return_stack = checkpoint.caller_return_stack;
        } else {
            cp_data.flag = _ctx.flag;
            cp_data.goingto_pc = _ctx.goingto_pc;
            cp_data.runtime_pc = _ctx.runtime_pc;
            cp_data.return_stack = _ctx.return_stack;
            _ctx.return_stack.clear();
        }
    } else {
        // Fresh main-program run: clear any stale breakpoint line from a
        // previous paused-inside-function stop.
        _ctx.last_break_line = 0;
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
            if (_ctx.debug_mode || _ctx.step_mode_active
                || _ctx.step_break_on_entry_pending) {
                _ctx.flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, true);
            } else {
                _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
            }
        }
    };

    auto return_stmt_id = stmt_id;

    while (prog_ptr != _prog_line.end()) {
        _ctx.runtime_pc.set(prog_ptr->first, return_stmt_id);

        const auto stmt = prog_ptr->second.first;
        const auto stmt_class = stmt->get_cl();
        const bool debug_steppable = stmt->is_debug_steppable();
        if (debug_function_call && !_ctx.call_stack.empty()) {
            debug_function_call_site_line
                = _ctx.call_stack.back().call_site_line;
        }

        auto& dbg = prog_ptr->second.second;
        const bool debug_checks_active = _ctx.debug_mode
            || _ctx.step_mode_active || _ctx.step_break_on_entry_pending
            || _ctx.flag[rt_prog_ctx_t::FLG_STOP_REQUEST] || dbg.break_point
            || dbg.single_step_break_point || dbg.continue_after_break;

        if (debug_checks_active) {
            const bool global_scope = _ctx.proc_scope.get_scope_id().empty();
            const bool global_proc_boundary = global_scope
                && (stmt_class == stmt_t::stmt_cl_t::SUB_BEGIN
                    || stmt_class == stmt_t::stmt_cl_t::SUB_END);

            if (_ctx.flag[rt_prog_ctx_t::FLG_STOP_REQUEST]) {
                dbginfo_t stop_dbg;
                stop_dbg.break_point = true;
                set_dbg_info(prog_ptr->first, stop_dbg);
                _ctx.flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, false);
            }

            const bool breakpoints_active = !_function_call || _ctx.debug_mode;

            if (_ctx.step_break_on_entry_pending && breakpoints_active
                && !global_proc_boundary
                && stmt_class != stmt_t::stmt_cl_t::EMPTY && debug_steppable) {
                _ctx.step_break_on_entry_pending = false;
                dbg.single_step_break_point = false;
                _ctx.last_break_line = prog_ptr->first;
                _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
                break;
            }

            if (dbg.single_step_break_point && breakpoints_active) {
                dbg.single_step_break_point = false;

                if (!global_proc_boundary && debug_steppable) {
                    _ctx.last_break_line = prog_ptr->first;
                    _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
                    break;
                }
            }

            if (dbg.break_point && breakpoints_active && !global_proc_boundary
                && debug_steppable) {
                if (dbg.condition_stmt != nullptr) {
                    dbg.condition_stmt->run(_ctx);
                } else {
                    _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
                }

                if (_ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]) {
                    _ctx.last_break_line = prog_ptr->first;
                    break;
                }
            } else if (dbg.continue_after_break) {
                // Reset breakpoint for next stmt execution
                dbg.break_point = true;
                dbg.continue_after_break = false;
            }

            if (!_ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]
                && _ctx.should_debug_stop_before_blocking_input()
                && source_line_has_blocking_input(
                    _ctx.get_source_line(prog_ptr->first))) {
                _ctx.debug_stop_before_blocking_input();
                break;
            }
        }


        // Skip execute empty stmt
        if (stmt_class == stmt_t::stmt_cl_t::EMPTY) {
            ++prog_ptr;
            if (prog_ptr != _prog_line.end() && _ctx.step_mode_active) {
                auto& breakp = prog_ptr->second.second;
                breakp.single_step_break_point = true;
            }
            continue;
        }

        // Run statement and update prog_ptr
        try {
            if (run_statement(stmt, return_stmt_id,
                    static_cast<prog_line_iterator_t&>(prog_ptr))) {
                return_stmt_id = 0;
            }
        } catch (const debug_suspend_t&) {
            _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
            break;
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

            // A procedure invoked directly as the program entry point returns
            // to line 0. This is valid for End Sub/End Function, but not for
            // a bare RETURN without a matching GOSUB.
            if (line == 0 && stmt_class == stmt_t::stmt_cl_t::SUB_END) {
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
                } else {
                    std::string err = "Runtime error.";
                    throw exception_t(err);
                }
            }

            if (jump == _prog_line.end()) {
                break;
            }

            prog_ptr = jump;
        } else if (_ctx.flag[rt_prog_ctx_t::FLG_JUMP_REQUEST]) {

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
                } else {
                    std::string err = "Runtime error.";
                    throw exception_t(err);
                }
            }

            prog_ptr = jump;

        } else if (_ctx.flag[rt_prog_ctx_t::FLG_SKIP_TILL_NEXT]) {

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
        if (end_flg && _ctx.debug_mode) {
            if (!using_debug_function_checkpoint) {
                rt_prog_ctx_t::debug_function_checkpoint_t checkpoint;
                checkpoint.function_name = function_name;
                checkpoint.expression_call = debug_function_call_site_line > 0;
                checkpoint.caller_flag = cp_data.flag;
                checkpoint.caller_runtime_pc = cp_data.runtime_pc;
                if (debug_function_call_site_line > 0) {
                    checkpoint.caller_runtime_pc.set(
                        debug_function_call_site_line, 0);
                }
                checkpoint.caller_goingto_pc = cp_data.goingto_pc;
                checkpoint.caller_return_stack = cp_data.return_stack;
                _ctx.debug_function_checkpoints.push_back(checkpoint);
            }

            _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
            return prog_ptr != _prog_line.end();
        }

        bool debug_checkpoint_completed = false;
        if (using_debug_function_checkpoint
            && !_ctx.debug_function_checkpoints.empty()) {
            const auto active_function
                = procedure_name_from_scope(_ctx.proc_scope.get_scope_id());
            debug_checkpoint_completed = active_function
                != _ctx.debug_function_checkpoints.front().function_name;
        }

        if (using_debug_function_checkpoint && !debug_checkpoint_completed) {
            _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, end_flg);
            return prog_ptr != _prog_line.end();
        }

        if (using_debug_function_checkpoint
            && !_ctx.debug_function_checkpoints.empty()
            && !_ctx.debug_function_checkpoints.front().expression_call) {
            _ctx.debug_function_checkpoints.pop_front();
            _ctx.last_break_line = 0;
            _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, end_flg);
            return prog_ptr != _prog_line.end();
        }

        _ctx.flag = cp_data.flag;
        _ctx.goingto_pc = cp_data.goingto_pc;
        _ctx.runtime_pc = cp_data.runtime_pc;
        _ctx.return_stack = cp_data.return_stack;

        if (using_debug_function_checkpoint
            && !_ctx.debug_function_checkpoints.empty()) {
            const auto checkpoint = _ctx.debug_function_checkpoints.front();
            _ctx.debug_function_checkpoints.pop_front();
            _ctx.queue_debug_pending_return(checkpoint.function_name,
                checkpoint.caller_runtime_pc.get_line());
        }

        _ctx.last_break_line = 0;

        // Propagate end-program flag
        _ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, end_flg);
    }

    return prog_ptr != _prog_line.end();
}


/* -------------------------------------------------------------------------- */

static bool run_function(program_t& program, rt_prog_ctx_t& ctx,
    const std::string& name, const std::vector<expr_any_t::handle_t>& args,
    bool& function_call, bool& debug_expression_call,
    bool is_debug_expression_call)
{
    const bool saved_function_call = function_call;
    const bool saved_debug_expression_call = debug_expression_call;
    if (is_debug_expression_call) {
        function_call = true;
    }
    debug_expression_call = is_debug_expression_call;
    struct function_call_guard_t {
        bool& function_call_value;
        bool saved_function_call_value;
        bool& debug_expression_call_value;
        bool saved_debug_expression_call_value;

        ~function_call_guard_t()
        {
            function_call_value = saved_function_call_value;
            debug_expression_call_value = saved_debug_expression_call_value;
        }
    } guard{ function_call, saved_function_call, debug_expression_call,
        saved_debug_expression_call };

    // Convert args in the stmt_call_t's argument list
    //
    arg_list_t arg_list;

    for (auto& arg : args) {
        variant_t value = arg->eval(ctx);

        expr_any_t::handle_t hvalue = std::make_shared<expr_literal_t>(value);

        arg_list.push_back(std::make_pair(
            hvalue, '\0' /*0 is a dummy value, unused in this context*/));
    }

    // Create a call-able object
    stmt_call_t call(arg_list, name, ctx, true);

    // Create a CALL-statement
    call.run(ctx, 0);

    // Retrieve the function prototype
    auto prototype = ctx.proc_prototypes.data.find(name);

    syntax_error_if(prototype == ctx.proc_prototypes.data.end(),
        "Cannot execute function " + name + ", prototype not found");

    // Finally executes the function
    return program.run(prototype->second.first.get_line());
}


/* -------------------------------------------------------------------------- */

bool program_t::run(
    const std::string& name, const std::vector<expr_any_t::handle_t>& args)
{
    return run_function(
        *this, _ctx, name, args, _function_call, _debug_expression_call, true);
}


/* -------------------------------------------------------------------------- */

bool program_t::run_entry_function(
    const std::string& name, const std::vector<expr_any_t::handle_t>& args)
{
    return run_function(
        *this, _ctx, name, args, _function_call, _debug_expression_call, false);
}


/* -------------------------------------------------------------------------- */

variant_t program_t::run_method(const std::string& name,
    const std::vector<expr_any_t::handle_t>& args,
    const std::string& me_obj_name, const variant_t& me_obj_value)
{
    const bool saved_function_call = _function_call;
    const bool saved_debug_expression_call = _debug_expression_call;
    _function_call = true;
    _debug_expression_call = true;
    struct function_call_guard_t {
        bool& function_call;
        bool saved_function_call;
        bool& debug_expression_call;
        bool saved_debug_expression_call;

        ~function_call_guard_t()
        {
            function_call = saved_function_call;
            debug_expression_call = saved_debug_expression_call;
        }
    } guard{ _function_call, saved_function_call, _debug_expression_call,
        saved_debug_expression_call };

    if (_ctx.debug_mode) {
        variant_t pending_return;
        if (_ctx.consume_debug_pending_return(
                name, _ctx.runtime_pc.get_line(), pending_return)) {
            return pending_return;
        }
    }

    // Convert args to arg_list_t
    arg_list_t arg_list;
    for (auto& arg : args) {
        variant_t value = arg->eval(_ctx);
        expr_any_t::handle_t hvalue = std::make_shared<expr_literal_t>(value);
        arg_list.push_back(std::make_pair(hvalue, '\0'));
    }

    // Enter callee scope and inject formal parameters
    stmt_call_t call(arg_list, name, _ctx, true);
    call.run(_ctx, 0);

    // Inject Me into callee scope with the pre-evaluated object value
    {
        auto callee_scope = _ctx.proc_scope.get();
        if (callee_scope)
            callee_scope->define(
                "me", var_value_t(me_obj_value, VAR_ACCESS_RW));
    }

    // Append Me writeback to the byref frame stmt_call_t already pushed
    if (!_ctx.byref_writeback_stack.empty())
        _ctx.byref_writeback_stack.back().emplace_back("me", me_obj_name);

    // Retrieve the prototype and run the function body
    auto prototype = _ctx.proc_prototypes.data.find(name);
    syntax_error_if(prototype == _ctx.proc_prototypes.data.end(),
        "Cannot execute method " + name + ", prototype not found");
    run(prototype->second.first.get_line());

    if (_ctx.debug_mode && _ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]) {
        throw debug_suspend_t();
    }

    // Retrieve and return the function return value
    auto it = _ctx.function_retval_tbl.find(name);
    if (it == _ctx.function_retval_tbl.end() || it->second.empty())
        return variant_t();
    auto ret = it->second.front();
    it->second.pop_front();
    if (it->second.empty())
        _ctx.function_retval_tbl.erase(it);
    return ret;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
