//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_MSGBOX_H__
#define __NU_MSGBOX_H__


/* -------------------------------------------------------------------------- */

#include "nu_window.h"
#include <gtk/gtk.h>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

int msgbox(
    GtkWindow *parentWindow,
    const char *text,
    const char *title = "", 
    GtkButtonsType btn = GTK_BUTTONS_OK)
{
    GtkWidget *dialog = gtk_message_dialog_new(
            parentWindow,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            btn,
            "%s",
            text);

    gtk_window_set_title(GTK_WINDOW(dialog), title);
    auto res = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    return res;
}


/* -------------------------------------------------------------------------- */

int msgbox(
    window_t window,
    const char *text,
    const char *title = "",
    GtkButtonsType btn = GTK_BUTTONS_OK)
{
    return msgbox((GtkWindow *) window.get_internal_obj(), text, title, btn);
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __MSGBOX_H__ 
