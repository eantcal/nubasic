//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//
#pragma once

/* -------------------------------------------------------------------------- */

#include "stdafx.h"
#include <string>

class txtinfobox_t;

/* -------------------------------------------------------------------------- */

//! Bottom panel with "Output" and "Console" tabs.
//
// Output tab  — txtinfobox_t (rich-edit) showing interpreter messages.
// Console tab — nuBASIC GDI console.  The console is created via
// nu_winconsole_init() so that nu_winconsole_write() /
// nu_winconsole_read_line() target the same window that is embedded here.

class tab_container_t {
public:
    enum class tab_id_t { OUTPUT = 0, CONSOLE = 1 };

    tab_container_t(HWND hwnd_parent, HINSTANCE hinstance);
    ~tab_container_t();

    // Non-copyable
    tab_container_t(const tab_container_t&) = delete;
    tab_container_t& operator=(const tab_container_t&) = delete;

    /* Layout */
    // Move / resize the whole panel.  Coordinates are in parent-client space.
    void arrange(WORD x_pos, WORD y_pos, WORD dx, WORD dy);

    /* Tab switching */
    void switch_to_tab(tab_id_t tab);
    tab_id_t get_current_tab() const { return _current_tab; }

    /* Access child controls */
    txtinfobox_t* get_infobox() { return _infobox; }

    /* WM_NOTIFY forwarding */
    void on_notify(LPNMHDR pnmhdr);

    /* Handles */
    HWND get_tab_hwnd() const { return _hwnd_tab; }

    bool is_console_detached() const { return _console_detached; }
    void toggle_console_detach();

private:
    void create_tab_items();
    void layout_content(const RECT& display_rc_in_parent);
    void update_visibility();
    // Returns the display area inside the tab strip, in PARENT-client
    // coordinates.
    RECT display_rect_in_parent() const;

    HWND _hwnd_parent;
    HWND _hwnd_tab;
    HINSTANCE _hinstance;

    txtinfobox_t* _infobox;

    HWND _hwnd_console; // owned by nu_winconsole_api
    bool _console_detached = false;

    tab_id_t _current_tab;
};

/* -------------------------------------------------------------------------- */
