//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
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

    statusbar_t( vbox_t& vbox, GtkWidget * statusbar = gtk_info_bar_new () )
    : 
    _statusbar(statusbar)
    {
      gtk_box_pack_start (GTK_BOX (vbox.get_internal_obj()), statusbar, FALSE, FALSE, 0);
      _label = gtk_label_new ("");

      gtk_box_pack_start (GTK_BOX (
        gtk_info_bar_get_content_area (GTK_INFO_BAR (_statusbar))), _label, FALSE, FALSE, 0);
    }

    void set_text( const char* text, GtkMessageType msgtype = GTK_MESSAGE_OTHER) const noexcept {
        gtk_info_bar_set_message_type (GTK_INFO_BAR (_statusbar), msgtype);
        gtk_label_set_text(GTK_LABEL(_label), text);
    }

    void hide() {
        gtk_widget_hide(_statusbar);
    }

    void show() {
        gtk_widget_show(_statusbar);
    }

private:
    GtkWidget * _statusbar = nullptr;
    GtkWidget * _label = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_STATUSBAR_H__ 
