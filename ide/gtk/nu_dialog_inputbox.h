//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_DIALOG_INPUTBOX_H__
#define __NU_DIALOG_INPUTBOX_H__


/* -------------------------------------------------------------------------- */

#include "nu_window.h"

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class dialog_inputbox_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _dialog_inputbox;
    }

    dialog_inputbox_t(
        const window_t& parent, 
        const char* title = "", 
        const char* label_text="",
        const char* edit_text="")
    {
        _dialog_inputbox = 
            gtk_dialog_new_with_buttons(title,
                    GTK_WINDOW(parent.get_internal_obj()),
                    (GtkDialogFlags) (GTK_DIALOG_MODAL
                            | GTK_DIALOG_DESTROY_WITH_PARENT),
                    GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
                    GTK_RESPONSE_CANCEL,
                    NULL);

        assert(_dialog_inputbox);

        auto hbox = gtk_hbox_new(FALSE, 8);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
        gtk_box_pack_start(
            GTK_BOX(gtk_dialog_get_content_area (GTK_DIALOG (_dialog_inputbox))), hbox,
            FALSE, FALSE, 0);

        auto stock = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
            GTK_ICON_SIZE_DIALOG);

        gtk_box_pack_start(GTK_BOX(hbox), stock, FALSE, FALSE, 0);

        auto table = gtk_table_new(1, 1, FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table), 4);
        gtk_table_set_col_spacings(GTK_TABLE(table), 4);
        gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 0);
        auto label = gtk_label_new_with_mnemonic(label_text);
        gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

        _entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(_entry), edit_text);

        gtk_table_attach_defaults(GTK_TABLE(table), _entry, 1, 2, 0, 1);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), _entry);

        gtk_widget_show_all(hbox);
    }

    GtkResponseType run() noexcept {
        auto resp = gtk_dialog_run(GTK_DIALOG(_dialog_inputbox));

        if (resp == GTK_RESPONSE_OK) {
            _edit_text = gtk_entry_get_text(GTK_ENTRY(_entry));
        }

        return GtkResponseType(resp);
    }

    const char* edit_text() const noexcept {
        return _edit_text.c_str();
    }

    virtual ~dialog_inputbox_t() {
        gtk_widget_destroy(_dialog_inputbox);
    }

private:
    GtkWidget * _entry;
    GtkWidget * _dialog_inputbox = nullptr;
    std::string _edit_text;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_DIALOG_INPUTBOX_H__ 
