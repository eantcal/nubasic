//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//
#pragma once

/* -------------------------------------------------------------------------- */

#include <cassert>
#include <gtk/gtk.h>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class menubar_t {
public:
    GtkWidget* get_internal_obj() const noexcept { return _menubar; }

    menubar_t(GtkWidget* menubar = gtk_menu_bar_new()) noexcept
        : _menubar(menubar)
    {
        assert(_menubar);
    }

    void set_sensitive(const bool& on) const noexcept
    {
        gtk_widget_set_sensitive(_menubar, on);
    }

private:
    GtkWidget* _menubar = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
