//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_MENU_H__
#define __NU_MENU_H__


/* -------------------------------------------------------------------------- */

#include "nu_accelgroup.h"
#include "nu_window.h"
#include "nu_menubar.h"

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class menu_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _menu;
    }

    menu_t(const char* name,
            const nu::menubar_t& menubar,
            const nu::accelgroup_t & accelgroup,
            GtkWidget * menu = gtk_menu_new())
    :
        _menu(menu),
        _accelgroup(accelgroup)
    {
        assert(_menu);

        auto menu_label = gtk_menu_item_new_with_label(name);

        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_label), _menu);

        gtk_menu_shell_append(
                GTK_MENU_SHELL(menubar.get_internal_obj()),
                menu_label);
    }

    template<class GtkCallbak_t>
    void add_stock_item(
            const window_t & window,
            const char* stock_id,
            GtkCallbak_t cbk)
    {
        add_item(window, stock_id, cbk, true);
    }

    template<class GtkCallbak_t>
    void add_item(
            const window_t & window,
            const char* id,
            GtkCallbak_t cbk,
            bool from_stock)
    {
        GtkWidget * item = from_stock ?
            gtk_image_menu_item_new_from_stock(
                                id,
                                _accelgroup.get_internal_obj()) :

            gtk_menu_item_new_with_label(id);

        gtk_menu_shell_append(GTK_MENU_SHELL(_menu), item);

        g_signal_connect(
                G_OBJECT (item),
                "activate",
                G_CALLBACK(cbk),
                (gpointer ) window.get_internal_obj());
    }

    void add_separator()
    {
        gtk_menu_shell_append(
                GTK_MENU_SHELL(_menu),
                gtk_separator_menu_item_new());
    }

private:
    GtkWidget * _menu = nullptr;
    const nu::accelgroup_t & _accelgroup;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_MENU_H__ 
