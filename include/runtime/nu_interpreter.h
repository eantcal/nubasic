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

#include "nu_basic_defs.h"
#include "nu_program.h"
#include "nu_statement_parser.h"
#include "nu_tokenizer.h"

#include <cstdio>
#include <map>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class source_line_t : public std::map<runnable_t::line_num_t, std::string> {};


/* -------------------------------------------------------------------------- */

class renum_tbl_t
    : public std::map<runnable_t::line_num_t, runnable_t::line_num_t> {};


/* -------------------------------------------------------------------------- */

class interpreter_t : protected signal_handler_t {
protected:
    enum { MAX_SRC_LINE_LEN = 1024 };

    volatile bool _break_event = false;
    volatile bool _ignore_break_event = false;

    interpreter_t(interpreter_t&) = delete;

    static std::string read_line(FILE* f);
    static std::string read_line(std::stringstream& ss);

    virtual bool notify(const event_t& ev) override;

    bool get_fileparameter(tokenizer_t& tknzr, std::string& filename);

public:
    rt_prog_ctx_t& get_rt_ctx() noexcept { return _prog_ctx; }
    void set_ignore_break_event(bool state) noexcept
    {
        _ignore_break_event = state;
    }
    bool ignore_break_event() const noexcept { return _ignore_break_event; }
    void register_break_event()
    {
        signal_mgr_t::instance().register_handler(event_t::BREAK, this);
    }

    struct breakpoint_cond_t {
        breakpoint_cond_t() = default;
        breakpoint_cond_t(const breakpoint_cond_t&) = default;
        breakpoint_cond_t& operator=(const breakpoint_cond_t&) = default;

        breakpoint_cond_t(
            const std::string& cond_str, stmt_t::handle_t cond_stmt)
            : condition_str(cond_str)
            , condition_stmt(cond_stmt)
        {
        }

        breakpoint_cond_t(breakpoint_cond_t&& other) noexcept
        {
            condition_str = std::move(other.condition_str);
            condition_stmt = std::move(other.condition_stmt);
        }

        breakpoint_cond_t& operator=(breakpoint_cond_t&& other) noexcept
        {
            if (this != &other) {
                condition_str = std::move(other.condition_str);
                condition_stmt = std::move(other.condition_stmt);
            }

            return *this;
        }

        std::string condition_str;
        nu::stmt_t::handle_t condition_stmt = nullptr;
    };

    using breakpoint_tbl_t
        = std::map<prog_pointer_t::line_number_t, breakpoint_cond_t>;

    interpreter_t(FILE* std_output = stdout, FILE* std_input = stdin);

    virtual ~interpreter_t();

    bool get_and_reset_break_event() noexcept
    {
        volatile bool res = _break_event;
        _break_event = false;
        return ignore_break_event() ? false : res;
    }

    enum class exec_res_t {
        NOP,
        SYNTAX_ERROR,
        IO_ERROR,
        RT_ERROR,
        UPDATE_PROG,
        CMD_EXEC,
        BREAKPOINT,
        STOP_REQ
    };


    exec_res_t set_step_mode(bool on);
    exec_res_t exec_command(const std::string& cmd);
    std::string version();

    prog_pointer_t::line_number_t get_cur_line_n() const noexcept;
    prog_pointer_t::line_number_t get_last_line_n() const noexcept;

    void renum_line(std::string& line, const renum_tbl_t& renum_tbl);

    bool is_breakpoint_active() const noexcept
    {
        return _breakpoints.find(_prog_ctx.runtime_pc.get_line())
            != _breakpoints.end();
    }

    bool is_stop_stmt_line() const noexcept
    {
        auto& stopmeta = _prog_ctx.stop_metadata.pc_stop_stmt;

        return stopmeta.find(_prog_ctx.runtime_pc.get_line()) != stopmeta.end();
    }

    void renum_prog(runnable_t::line_num_t step);
    bool update_program(const std::string& line, int ln = 0);

    void clear_all();

    bool has_runnable_stmt(int line) const noexcept;
    bool get_global_var(const std::string& name, nu::variant_t& var);
    bool set_global_var(const std::string& name, const nu::variant_t& value);

    void clear_rtdata();

    void rebuild(runnable_t::line_num_t to_num = 0 /* all */);
    void erase_line(runnable_t::line_num_t num);
    bool run(runnable_t::line_num_t start_from = 0);
    bool run_next(runnable_t::line_num_t line);
    bool cont(runnable_t::line_num_t start_from, runnable_t::stmt_num_t stmtid);

    // Run main() if defined, otherwise run from line 0.
    bool run_main_or_default();

    exec_res_t set_breakpoint(
        runnable_t::line_num_t line, breakpoint_cond_t&& bp);
    exec_res_t erase_breakpoint(runnable_t::line_num_t line);
    bool continue_afterbrk(runnable_t::line_num_t line);

    bool load(FILE* f);
    bool load(const std::string& filepath);
    bool load(std::stringstream& source, const std::string& base_dir = "");
    bool append(std::stringstream& is, int& n_of_lines);

    bool save(const std::string& filepath);

    bool list(runnable_t::line_num_t from = 0, runnable_t::line_num_t to = 0,
        const std::string grep_filter = "");

    static std::string skip_blank(expr_tknzr_t& tz, nu::token_t& t);

    exec_res_t break_if(prog_pointer_t::line_number_t line, token_list_t& tl);

    FILE* get_stdout_ptr() const noexcept { return _stdout_ptr; }
    FILE* get_stdin_ptr() const noexcept { return _stdin_ptr; }

    // Store CLI arguments passed to the program's main() entry point.
    // argv[0] is conventionally the script filename; argv[1..] are script args.
    void set_cli_args(const std::vector<std::string>& args) noexcept
    {
        _cli_args = args;
    }

    void set_yield_cbk(
        program_t::yield_cbk_t cbk, void* cbk_data = nullptr) noexcept
    {
        _yield_cbk = cbk;
        _yield_data = cbk_data;

        if (_prog) {
            _prog->set_yield_cbk(cbk, cbk_data);
        }
    }

protected:
    program_t* _prog = nullptr;

    // Recursive helper for load() that handles #Include / Include directives.
    // base_dir: directory of the including file (used for relative paths).
    // ln: sequential line counter, shared across recursive calls.
    // depth: recursion guard (max 64 levels).
    bool load_with_includes(
        FILE* f, int& ln, const std::string& base_dir, int depth = 0);
    bool load_with_includes(std::stringstream& source, int& ln,
        const std::string& base_dir, int depth = 0);

    program_t::yield_cbk_t _yield_cbk = nullptr;
    void* _yield_data = nullptr;

private:
    breakpoint_tbl_t _breakpoints;

    prog_line_t _prog_line;
    source_line_t _source_line;
    rt_prog_ctx_t _prog_ctx;
    nu::statement_parser_t _parser;
    FILE* _stdout_ptr;
    FILE* _stdin_ptr;

    // Arguments passed to the program's main() entry point.
    std::vector<std::string> _cli_args;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
