//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_PROG_POINTER_H__
#define __NU_PROG_POINTER_H__


/* -------------------------------------------------------------------------- */

#include <deque>
#include <sstream>

#include "nu_cpp_lang.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_pointer_t {
public:
    using line_number_t = int;
    using stmt_number_t = int;

    prog_pointer_t(line_number_t line = 0, stmt_number_t stmt = 0) noexcept
        : _line(line),
          _line_stmt(stmt)
    {
    }

    prog_pointer_t(const prog_pointer_t&) = default;
    prog_pointer_t& operator=(const prog_pointer_t&) = default;


    void go_to(line_number_t line, stmt_number_t stmt = 0) noexcept
    {
        set(line, stmt);
    }


    void set(line_number_t line, stmt_number_t stmt) noexcept
    {
        _last_line = _line;
        _line = line;
        _line_stmt = stmt;
    }


    void set_stmt_pos(stmt_number_t stmt) { _line_stmt = stmt; }


    line_number_t get_line() const noexcept { return _line; }


    line_number_t get_last_line() const noexcept { return _last_line; }


    int get_stmt_pos() const noexcept { return _line_stmt; }


    void reset() noexcept { set(0, 0); }


    bool operator<(const prog_pointer_t& pp) const noexcept
    {
        return _line > pp._line
            || (_line == pp._line && _line_stmt > pp._line_stmt);
    }


    std::string to_string() const noexcept
    {
        return std::to_string(_line) + ":" + std::to_string(_line_stmt);
    }

private:
    line_number_t _line = 0;
    stmt_number_t _line_stmt = 0;
    stmt_number_t _last_line = 0;
};


/* -------------------------------------------------------------------------- */

struct return_stack_t
    : public std::deque<std::pair<prog_pointer_t::line_number_t,
          prog_pointer_t::stmt_number_t>> {
    void trace(std::stringstream& ss);
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_PROG_POINTER_H__
