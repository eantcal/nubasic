//  
// This file is part of nuBASIC IDE Project
// Copyright (c) 2017 - Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_TOOLBAR_H__
#define __NU_TOOLBAR_H__


/* -------------------------------------------------------------------------- */

#include "nu_window.h"

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class toolbar_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _toolbar;
    }

    toolbar_t( GtkWidget * toolbar = gtk_toolbar_new() )
    :
        _toolbar(toolbar)
    {
        assert(_toolbar);
    }

    template<class GtkCallbak_t>
    void add_stock_item(
            const char* stock_id, 
            const char* tip,
            const window_t& window, 
            GtkCallbak_t cbk, 
            int button_id)
    {
        GtkToolItem * tool_item = gtk_tool_button_new_from_stock(stock_id);

        gtk_toolbar_insert(GTK_TOOLBAR(_toolbar), tool_item, button_id);
        gtk_widget_set_tooltip_text(GTK_WIDGET(tool_item), tip);

        g_signal_connect(G_OBJECT (tool_item), "clicked", G_CALLBACK (cbk),
                (gpointer) window.get_internal_obj());
    }

    void set_show_arrow( bool on = true ) const noexcept {
        gtk_toolbar_set_show_arrow(GTK_TOOLBAR(_toolbar), on ? TRUE : FALSE);
    }

    void set_style( GtkToolbarStyle style = GTK_TOOLBAR_ICONS ) const noexcept {
        gtk_toolbar_set_style(GTK_TOOLBAR(_toolbar), style);
    }


private:
    GtkWidget * _toolbar = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_TOOLBAR_H__ 
