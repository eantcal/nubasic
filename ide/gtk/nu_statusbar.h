//  
// This file is part of nuBASIC IDE Project
// Copyright (c) 2017 - Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_STATUSBAR_H__
#define __NU_STATUSBAR_H__


/* -------------------------------------------------------------------------- */

#include "nu_vbox.h"

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class statusbar_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _statusbar;
    }

    statusbar_t( vbox_t& vbox, GtkWidget * statusbar = gtk_statusbar_new() )
    :
        _statusbar(statusbar)
    {
        gtk_box_pack_start (GTK_BOX (vbox.get_internal_obj()), _statusbar, FALSE, TRUE, 0);
        gtk_widget_show (_statusbar);

        _label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(_statusbar), _label, FALSE, TRUE, 0);

    }

    void set_text( const char* text ) const noexcept {
        gtk_label_set_text(GTK_LABEL(_label), text);
    }

private:
    GtkWidget * _statusbar = nullptr;
    GtkWidget * _label = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_STATUSBAR_H__ 
