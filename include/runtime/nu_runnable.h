//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_RUNNABLE_H__
#define __NU_RUNNABLE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_pointer.h"
#include "nu_signal_handling.h"

#include <cstdio>
#include <map>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class runnable_t : protected signal_handler_t {
public:
    using line_num_t = prog_pointer_t::line_number_t;
    using stmt_num_t = prog_pointer_t::stmt_number_t;
    
    runnable_t() {
        signal_mgr_t::instance().register_handler(event_t::BREAK, this);
    }


    virtual ~runnable_t() {
        signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
    }


    virtual bool run(line_num_t start_from = 0) = 0;
    virtual bool cont(line_num_t start_from, stmt_num_t stmt_id) = 0;
    virtual bool run(
        const std::string& name, const std::vector<expr_any_t::handle_t>& args)
        = 0;

protected:
    bool notify(const event_t& ev) override {
        _break_event = ev == event_t::BREAK;
        return true;
    }

    bool break_event() const noexcept { 
        return _break_event; 
    }
    
    void reset_break_event() noexcept { 
        _break_event = false; 
    }
    
    void set_break_event() noexcept { 
        _break_event = true; 
    }

private:
    bool _break_event = false;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_RUNNABLE_H__
