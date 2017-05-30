//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_DIALOG_ABOUT_H__
#define __NU_DIALOG_ABOUT_H__


/* -------------------------------------------------------------------------- */

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class dialog_about_t
{
public:
    dialog_about_t(
        const char* prog, 
        const char* ver, 
        const char* auth, 
        const char* lic,
        const char* desc)
    {
	    auto about = gtk_about_dialog_new();

	    gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about), prog);
	    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), ver);

	    auto text = g_strdup_printf("Written by %s\nLicensed under %s", auth, lic);
	    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about), text);
	    g_free(text);

		gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about), desc);

	    // gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), icon);

	    gtk_dialog_run(GTK_DIALOG(about));
	    gtk_widget_destroy(about);
    }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_DIALOG_ABOUT_H__ 
