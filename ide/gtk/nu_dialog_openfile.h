//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//
#pragma once

/* -------------------------------------------------------------------------- */

#include "nu_window.h"

#include <cassert>
#include <gtk/gtk.h>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class dialog_openfile_t {
public:
    GtkWidget* get_internal_obj() const noexcept { return _dialog_openfile; }

    dialog_openfile_t(const window_t& parent, const char* title = "")
    {
        _dialog_openfile = gtk_file_chooser_dialog_new(title,
            GTK_WINDOW(parent.get_internal_obj()), GTK_FILE_CHOOSER_ACTION_OPEN,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
            GTK_RESPONSE_ACCEPT, NULL);

        assert(_dialog_openfile);
    }

    GtkResponseType run() noexcept
    {

        auto resp = gtk_dialog_run(GTK_DIALOG(_dialog_openfile));

        if (resp == GTK_RESPONSE_ACCEPT) {
            if (_filename) {
                g_free(_filename);
            }

            _filename = gtk_file_chooser_get_filename(
                GTK_FILE_CHOOSER(_dialog_openfile));
        }

        return GtkResponseType(resp);
    }

    const char* filename() const noexcept { return _filename; }

    virtual ~dialog_openfile_t()
    {
        if (_filename) {
            g_free(_filename);
        }

        gtk_widget_destroy(_dialog_openfile);
    }

private:
    GtkWidget* _dialog_openfile = nullptr;
    char* _filename = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
