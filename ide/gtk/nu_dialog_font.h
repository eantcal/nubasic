//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_DIALOG_FONT_H__
#define __NU_DIALOG_FONT_H__


/* -------------------------------------------------------------------------- */

#include "nu_window.h"

#include <gtk/gtk.h>
#include <pango/pangocairo.h>
#include <cassert>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class dialog_font_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return GTK_WIDGET(_dialog_font);
    }

    dialog_font_t(
            const window_t& parent,
            const char* font,
            const int size,
            const char* title = "Select Font")
    {
        _dialog_font =
           GTK_FONT_SELECTION_DIALOG(gtk_font_selection_dialog_new(title));

        std::string s_font = font ? font : "Monospace";
        s_font += " ";
        s_font += size > 0 ? std::to_string(size) : "9";

        gtk_font_selection_dialog_set_font_name(_dialog_font, s_font.c_str());
        gtk_font_selection_dialog_set_preview_text(_dialog_font,"ABC abc 012");

        gtk_widget_show(GTK_WIDGET(_dialog_font));

        assert(_dialog_font);
    }

    GtkResponseType run() noexcept {

        auto resp = gtk_dialog_run(GTK_DIALOG(_dialog_font));

        if ( resp == GTK_RESPONSE_OK) {
            auto font = gtk_font_selection_dialog_get_font_name(_dialog_font);

            if (font) {
                auto desc = pango_font_description_from_string(font);

                if (desc) {
                    _font = font; //pango_font_description_get_family (desc);
                    _font_size = pango_font_description_get_size(desc) /  PANGO_SCALE;
                    auto s = std::to_string(_font_size);
                    const auto ss = s.size() + 1;
                    if (_font.size()>ss) {
                        _font = _font.substr(0,_font.size()-ss);
                    }
                }

                pango_font_description_free(desc);
            }
        }

        else if ( resp == GTK_RESPONSE_CANCEL) {
            // TODO
        }

        gtk_widget_hide(GTK_WIDGET(_dialog_font));

        return GtkResponseType(resp);
    }

    const char* font() const noexcept {
        return _font.c_str();
    }

    const int font_size() const noexcept {
        return _font_size;
    }

    virtual ~dialog_font_t() {
        gtk_widget_destroy(GTK_WIDGET(_dialog_font));
    }

private:
    GtkFontSelectionDialog * _dialog_font = nullptr;
    std::string _font;
    int _font_size = 0;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_DIALOG_FONT_H__ 
