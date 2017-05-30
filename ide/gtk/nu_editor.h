//  
// This file is part of nuBASIC IDE Project
// Copyright (c) 2017 - Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_EDITOR_H__
#define __NU_EDITOR_H__


/* -------------------------------------------------------------------------- */

#include <gtk/gtk.h>
#include <cassert>

#include "Scintilla.h"
#include "SciLexer.h"
#define PLAT_GTK 1
#include "ScintillaWidget.h"


#define RGB(r,g,b) \
    ((unsigned int)(((unsigned char)(r)|\
    ((unsigned short)((unsigned char)(g))<<8))|\
    (((unsigned int)((unsigned char)(b)))<<16)))

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class editor_t
{
public:
    enum class marker_t {
       BOOKMARK = 0,
       BREAKPOINT = 1,
       PROGCOUNTER = 2,
       LINESELECTION = 4,
    };

    enum {
       LINENUM_WIDTH = 5,
       DEF_TABWIDTH = 3,
       DEF_INDENT = DEF_TABWIDTH,
       DEF_MARGIN_WIDTH = 16,
       DEF_CARETPERIOD = 400 // ms
    };


    GtkWidget * get_internal_obj() const noexcept {
        return _editor;
    }

    explicit editor_t(GtkWidget* editor = scintilla_new()) :
        _editor(editor)
    {
        assert(_editor);
    }

    template <class Notify>
    void set_notify_cbk(Notify cbk) const noexcept {
        g_signal_connect(_editor, "sci-notify", G_CALLBACK(cbk), NULL);
    }

    void set_id(int id = 0) const noexcept {
        scintilla_set_id(SCINTILLA(_editor), id);
    }

    template<class M, class W, class L>
    sptr_t cmd(M m, W w, L l) const noexcept {
        return scintilla_send_message(SCINTILLA(_editor), m, (sptr_t) w, (sptr_t) l);
    }

    template<class M, class W>
    sptr_t cmd(M m, W w) const noexcept {
        return scintilla_send_message(SCINTILLA(_editor), m, (sptr_t) w, (sptr_t) 0);
    }

    template<class M>
    sptr_t cmd(M m) const noexcept {
        return scintilla_send_message(SCINTILLA(_editor), m, (sptr_t) 0, (sptr_t) 0);
    }

    void update_ui() const noexcept {
        long begin = (long)cmd(SCI_GETCURRENTPOS, 0, 0);
        long end = (long)cmd(SCI_BRACEMATCH, begin - 1, 0);

        if (end < 0) {
            begin = 0;
            end = -1;
        }

        cmd(SCI_BRACEHIGHLIGHT, begin - 1, end);
    }

    void set_def_folding(int margin, long pos) const noexcept {
        // simply toggle fold
        if (margin == 2) {
            cmd(SCI_TOGGLEFOLD, cmd(SCI_LINEFROMPOSITION, pos, 0), 0);
        }
    }

    int get_line_count() const noexcept {
        return int((cmd(SCI_GETLINECOUNT, 0, 0)));
    }

    void set_folding_margin(bool enable) noexcept {
        cmd(SCI_SETMARGINWIDTHN, 2, enable ? DEF_MARGIN_WIDTH : 0);
    }

    int get_line_num_width() const noexcept {
        return int(LINENUM_WIDTH *
                int(cmd(SCI_TEXTWIDTH, STYLE_LINENUMBER, (sptr_t)("9"))));
    }

    void set_numbers_margin(bool enable) const noexcept {
        cmd(SCI_SETMARGINWIDTHN, 0, enable ? get_line_num_width() + 4 : 0);
    }

    void set_selection_margin(bool enable) const noexcept {
        cmd(SCI_SETMARGINWIDTHN, 1, enable ? DEF_MARGIN_WIDTH : 0);
    }

    bool add_bookmark(long line) const noexcept {
        if (!has_bookmark(line)) {
            const auto m = int(marker_t::BOOKMARK);

            cmd(SCI_MARKERDEFINE, m, SC_MARK_BOOKMARK);
            cmd(SCI_MARKERSETFORE, m, RGB(0, 0, 0));
            cmd(SCI_MARKERSETBACK, m, RGB(0, 255, 0));

            cmd(SCI_MARKERADD, line - 1, m);
            return true;
        }

        return false;
    }

    void toggle_bookmark(long line) const noexcept {
        if (!add_bookmark(line))
            remove_bookmark(line);
    }


    void remove_prog_cnt_marker() const noexcept {
        cmd(SCI_MARKERDELETEALL, int(marker_t::PROGCOUNTER), 0);
        cmd(SCI_MARKERDELETEALL, int(marker_t::LINESELECTION), 0);
        cmd(SCI_LINESCROLLDOWN, 0, 0);
        cmd(SCI_LINESCROLLUP, 0, 0);
    }

    bool remove_bookmark(long line) const noexcept  {
        if (has_bookmark(line)) {
            cmd(SCI_MARKERDELETE, line - 1, 0);
            return true;
        }

        return false;
    }

    bool add_breakpoint(long line) const noexcept {
        const auto m = int(marker_t::BREAKPOINT);
        cmd(SCI_MARKERDEFINE, m, SC_MARK_CIRCLE);
        cmd(SCI_MARKERSETFORE, m, RGB(255, 255, 255));
        cmd(SCI_MARKERSETBACK, m, RGB(255, 0, 0));

        cmd(SCI_MARKERADD, line - 1, m);

        return true;
    }

    bool toggle_breakpoint(long line) const noexcept {
        if (!remove_breakpoint(line))
            add_breakpoint(line);

        return true;
    }

    bool remove_breakpoint(long line) const noexcept {
        if (has_breakpoint(line)) {
            cmd(SCI_MARKERDELETE, line - 1, 1);
            return true;
        }

        return false;
    }

    void remove_all_bookmarks() const noexcept {
        cmd(SCI_MARKERDELETEALL, int(marker_t::BOOKMARK), 0);
    }

    void remove_all_breakpoints() const noexcept {
        cmd(SCI_MARKERDELETEALL, int(marker_t::BREAKPOINT), 0);
    }

    bool has_bookmark(long line) const noexcept {
        const auto m = int(marker_t::BOOKMARK) + 1;
        return ((cmd(SCI_MARKERGET, line - 1, 0) & m) == m);
    }

    bool has_breakpoint(long line) const noexcept {
        const auto m = int(marker_t::BREAKPOINT) + 1;
        return ((cmd(SCI_MARKERGET, line - 1, 0) & m) == m);
    }

    void set_font(int style, const char* name) const noexcept {
        cmd(SCI_STYLESETFONT, style, name);
    }

    void set_font_height(int style, int height) const noexcept {
        cmd(SCI_STYLESETSIZE, style, height);
    }

    void set_fg(int style, unsigned long crForeground) const noexcept {
        cmd(SCI_STYLESETFORE, style, crForeground);
    }

    void set_bg(int style, unsigned long crBackground) const noexcept {
        cmd(SCI_STYLESETBACK, style, crBackground);
    }

    void set_font_bold(int style, bool enable) const noexcept {
        cmd(SCI_STYLESETBOLD, style, enable);
    }

    void set_font_italic(int style, bool enable) const noexcept {
        cmd(SCI_STYLESETITALIC, style, enable);
    }

    void set_font_underline(int style, bool enable) noexcept {
        cmd(SCI_STYLESETUNDERLINE, style, enable);
    }

    bool get_overstrike() const noexcept {
        return cmd(SCI_GETOVERTYPE, 0, 0) != 0;
    }

    void set_overstrike(bool enable) const noexcept {
        cmd(SCI_SETOVERTYPE, enable ? TRUE : FALSE, 0);
    }

    void go_to_line(long line) const noexcept {
        cmd(SCI_GOTOLINE, line - 1, 0);
    }

    int get_search_flags() const noexcept {
        return int(cmd(SCI_GETSEARCHFLAGS, 0, 0));
    }

    void set_search_flags(int flags) noexcept {
        cmd(SCI_SETSEARCHFLAGS, flags, 0);
    }

    void refresh() const noexcept {
        cmd(SCI_COLOURISE, 0, -1);
    }

    int get_selection_begin() const noexcept {
        return int(cmd(SCI_GETSELECTIONSTART, 0, 0));
    }

    int get_selection_end() const noexcept {
        return int(cmd(SCI_GETSELECTIONEND, 0, 0));
    }

    int get_current_line() const noexcept {
        return int(cmd(SCI_LINEFROMPOSITION, int(cmd(SCI_GETCURRENTPOS, 0, 0)), 0)) + 1;
    }

    int get_current_colum() const noexcept {
        return int(cmd(SCI_GETCOLUMN, int(cmd(SCI_GETCURRENTPOS, 0, 0)), 0)) + 1;
    }

    int get_current_position() const noexcept {
        return int(cmd(SCI_GETCURRENTPOS, 0, 0));
    }

    int get_current_style() const noexcept {
        return int(cmd(SCI_GETSTYLEAT, get_current_position(), 0));
    }

    int get_fold_level() const noexcept {
        int level = int(cmd(SCI_GETFOLDLEVEL, get_current_line(), 0))
            & SC_FOLDLEVELNUMBERMASK;

        return level - 1024;
    }

    void find_next_bookmark() const noexcept {
        auto line = int(cmd(SCI_MARKERNEXT, get_current_line(), 0xffff));

        if (line >= 0)
            cmd(SCI_GOTOLINE, line, 0);
    }

    void find_prev_bookmark() const noexcept {
        auto line
            = int(cmd(SCI_MARKERPREVIOUS, get_current_line() - 2, 0xffff));

        if (line >= 0)
            cmd(SCI_GOTOLINE, line, 0);
    }

    void set_item_style(
        int style,
        int fore,
        int back = 0xffffff,
        int size = 0,
        const char* face = nullptr) const noexcept
    {
        cmd(SCI_STYLESETFORE, style, fore);
        cmd(SCI_STYLESETBACK, style, back);

        if (size >= 1)
            cmd(SCI_STYLESETSIZE, style, size);

        if (face)
            cmd(SCI_STYLESETFONT, style, face);
    }

    /* -------------------------------------------------------------------------- */

    void get_text_range(int start, int end, char* text) const noexcept
    {
        Sci_TextRange tr = { 0 };

        tr.chrg.cpMin = start;
        tr.chrg.cpMax = end;
        tr.lpstrText = text;

        cmd(SCI_GETTEXTRANGE, 0, &tr);
    }



private:
    GtkWidget * _editor = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_EDITOR_H__ 
