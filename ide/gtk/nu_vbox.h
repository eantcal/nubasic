//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_VBOX_H__
#define __NU_VBOX_H__


/* -------------------------------------------------------------------------- */

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class vbox_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _vbox;
    }

    vbox_t(GtkWidget* vbox = gtk_vbox_new(FALSE, 0)) :
        _vbox(vbox)
    {
        assert(_vbox);
    }

    template<class Child>
    void pack_start(
        const Child& child,
        int padding = 2,
        bool expand = false,
        bool fill = false) const noexcept
    {
        gtk_box_pack_start(
                GTK_BOX(_vbox),
                child.get_internal_obj(),
                expand ? TRUE : FALSE,
                fill ? TRUE : FALSE,
                padding);
    }

    template<class Container>
    void container_add(const Container& container) const noexcept {
        gtk_container_add(GTK_CONTAINER(_vbox), container.get_internal_obj());
    }

    template<class CallBack>
    void signal_connect(const char* event, CallBack cbk) {
        g_signal_connect(G_OBJECT (_vbox), event, G_CALLBACK (cbk), NULL);
    }

    template<class CallBack>
    void on_destroy(CallBack cbk) {
        signal_connect<CallBack>("destroy", cbk);
    }

    template<class CallBack>
    void on_delete_event(CallBack cbk) {
        signal_connect<CallBack>("delete_event", cbk);
    }

private:
    GtkWidget * _vbox = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_VBOX_H__ 
