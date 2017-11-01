//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_SIGNAL_HANDLING_H__
#define __NU_SIGNAL_HANDLING_H__


/* -------------------------------------------------------------------------- */

#include <cassert>
#include <map>
#include <memory>
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct signal_handler_t {
    using handle_t = signal_handler_t*;

    // Right now we've got just an event
    enum class event_t { BREAK };

    virtual bool notify(const event_t& ev) = 0;
    virtual ~signal_handler_t() {}
};


/* -------------------------------------------------------------------------- */

class signal_mgr_t {
private:
    // This is a singleton
    static signal_mgr_t* _instance;

    signal_mgr_t() = default;

    // Copy not allowed
    signal_mgr_t(const signal_mgr_t&) = delete;
    signal_mgr_t& operator=(const signal_mgr_t&) = delete;

public:
    static signal_mgr_t& instance();


    void register_handler(
        const signal_handler_t::event_t& ev, signal_handler_t::handle_t handle)
    {
        _handlers[ev].insert(handle);
    }


    void unregister_handler(
        const signal_handler_t::event_t& ev, signal_handler_t::handle_t handle)
    {
        auto i = _handlers.find(ev);

        if (i != _handlers.end())
            i->second.erase(handle);
    }
    
    void disable_notifications() throw() { 
        _disable_notifications = true; 
    }
    
    void enable_notifications() throw() { 
        _disable_notifications = false; 
    }
    
protected:
    using hlist_t = std::set<signal_handler_t::handle_t>;
    std::map<signal_handler_t::event_t, hlist_t> _handlers;
    bool dispatch(signal_handler_t::event_t ev);
    friend bool _ev_dispatcher(signal_handler_t::event_t ev);
    bool _disable_notifications = false;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_SIGNAL_HANDLING_H__
