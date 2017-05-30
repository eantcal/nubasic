//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_WIDGET_H__
#define __NU_WIDGET_H__


/* -------------------------------------------------------------------------- */

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

template <class Widget>
class widget_t
{
public:
    widget_t( const Widget& widget ) noexcept : 
        _widget(widget) 
    {}

    GtkWidget * get_internal_obj() const noexcept {
        return _widget.get_internal_obj();
    }

    void show_all() const noexcept {
        gtk_widget_show_all(GTK_WIDGET(get_internal_obj()));
    }

    void grab_focus() const noexcept {
        gtk_widget_grab_focus(GTK_WIDGET(get_internal_obj()));
    }

private:
    const Widget& _widget;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_WIDGET_H__ 
