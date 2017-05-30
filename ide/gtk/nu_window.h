//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_WINDOW_H__
#define __NU_WINDOW_H__


/* -------------------------------------------------------------------------- */

#include "nu_accelgroup.h"

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


class window_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _window;
    }

    explicit window_t(GtkWindowType type = GTK_WINDOW_POPUP) :  //GTK_WINDOW_TOPLEVEL
      _window(gtk_window_new(type))
    {
        assert(_window);
    }

    explicit window_t(GtkWidget * window) : _window(window) {
        assert(window);
    }
   
    void set_title(const char* title) const noexcept {
        gtk_window_set_title(GTK_WINDOW(_window), title);
    }

    void default_size(int dx, int dy) const noexcept {
        gtk_window_set_default_size(GTK_WINDOW(_window), dx, dy);
    }

    void maximize() const noexcept {
        gtk_window_maximize(GTK_WINDOW(_window));
    }

    void add_accel_group(const accelgroup_t& ag) const noexcept {
        gtk_window_add_accel_group(GTK_WINDOW(_window), ag.get_internal_obj());
    }

    template<class CallBack>
    void signal_connect(const char* event, CallBack cbk) {
        g_signal_connect(G_OBJECT (_window), event, G_CALLBACK (cbk), NULL);
    }

    template<class CallBack>
    void on_destroy(CallBack cbk) {
        signal_connect<CallBack>("destroy", cbk);
    }

    template<class CallBack>
    void on_delete_event(CallBack cbk) {
        signal_connect<CallBack>("delete_event", cbk);
    }

    template<class Widget>
    void add_container(const Widget& widget) {
        gtk_container_add(
            GTK_CONTAINER(_window),
            widget.get_internal_obj());
    }

private:
    GtkWidget * _window = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_WINDOW_H__ 
