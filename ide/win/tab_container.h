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
#include <utility>
#include <vector>

class txtinfobox_t;

/* -------------------------------------------------------------------------- */

//! Bottom panel with "Output", "Console" and "Call Stack" tabs.
//
// Output tab     — txtinfobox_t (rich-edit) showing interpreter messages.
// Console tab    — nuBASIC GDI console.
// Call Stack tab — read-only text showing the debug call stack (debug_mode
// only).

class tab_container_t {
public:
    enum class tab_id_t { OUTPUT = 0, CONSOLE = 1, CALLSTACK = 2 };

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

    /* Call Stack tab — frames as (func_name, call_site_line) pairs */
    void update_call_stack(
        const std::vector<std::pair<std::string, int>>& frames);
    void clear_call_stack();

    /* WM_NOTIFY forwarding */
    void on_notify(LPNMHDR pnmhdr);

    /* Handles */
    HWND get_tab_hwnd() const { return _hwnd_tab; }

    bool is_console_detached() const { return _console_detached; }
    void toggle_console_detach();

private:
    void create_tab_items();
    void insert_tab_item(int index, const wchar_t* text, tab_id_t id);
    int tab_index_of(tab_id_t tab) const;
    tab_id_t tab_id_at(int sel) const;
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
    HWND _hwnd_callstack = nullptr; // Read-only text control for Call Stack tab
    bool _console_detached = false;

    tab_id_t _current_tab;
};

/* -------------------------------------------------------------------------- */
