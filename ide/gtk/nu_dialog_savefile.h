//  
// This file is part of nuBASIC IDE Project
// Copyright (c) 2017 - Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_DIALOG_SAVEFILE_H__
#define __NU_DIALOG_SAVEFILE_H__


/* -------------------------------------------------------------------------- */

#include "nu_window.h"

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class dialog_savefile_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _dialog_savefile;
    }

    dialog_savefile_t(
        const window_t& parent, 
        const char* title = "")
    {
        _dialog_savefile = gtk_file_chooser_dialog_new(
            title, 
            GTK_WINDOW(parent.get_internal_obj()),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            GTK_STOCK_CANCEL, 
            GTK_RESPONSE_CANCEL,
            GTK_STOCK_SAVE, 
            GTK_RESPONSE_ACCEPT,
            NULL);

        assert(_dialog_savefile);

        gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (_dialog_savefile), TRUE);
    }

    GtkResponseType run(
        const char* path = nullptr, 
        const char* name = nullptr) noexcept 
    {
        if (path && name) {
            gtk_file_chooser_set_current_folder (
                GTK_FILE_CHOOSER (_dialog_savefile), path);

            gtk_file_chooser_set_current_name (
                GTK_FILE_CHOOSER (_dialog_savefile), name);
        }
        else {
            gtk_file_chooser_set_filename (
                GTK_FILE_CHOOSER (_dialog_savefile), _filename ? _filename : "");
        }

        auto resp = gtk_dialog_run(GTK_DIALOG(_dialog_savefile));

        if (resp == GTK_RESPONSE_ACCEPT) {
            if (_filename) 
                g_free(_filename);

            _filename = gtk_file_chooser_get_filename(
                    GTK_FILE_CHOOSER(_dialog_savefile));
        }

        return GtkResponseType(resp);
    }

    const char* filename() const noexcept {
        return _filename;
    }

    virtual ~dialog_savefile_t() {
        if (_filename) 
            g_free(_filename);

        gtk_widget_destroy(_dialog_savefile);
    }

private:
    GtkWidget * _dialog_savefile = nullptr;
    char * _filename = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_DIALOG_SAVEFILE_H__
