//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "tab_container.h"
#include "nu_winconsole_api.h"
#include "textinfobox.h"

#include <cassert>
#include <commctrl.h>

/* -------------------------------------------------------------------------- */

tab_container_t::tab_container_t(HWND hwnd_parent, HINSTANCE hinstance)
    : _hwnd_parent(hwnd_parent)
    , _hinstance(hinstance)
    , _hwnd_tab(nullptr)
    , _infobox(nullptr)
    , _hwnd_console(nullptr)
    , _current_tab(tab_id_t::OUTPUT)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    // ---- Tab control -------------------------------------------------------
    _hwnd_tab = CreateWindowW(WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_BOTTOM, 0, 0, 0, 0,
        _hwnd_parent, nullptr, _hinstance, nullptr);
    assert(_hwnd_tab);

    create_tab_items();

    // ---- Output infobox (rich-edit) ----------------------------------------
    _infobox = new txtinfobox_t(_hwnd_parent, _hinstance);

    // ---- Call Stack ListView -----------------------------------------------
    _hwnd_callstack = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
        WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL
            | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
        0, 0, 0, 0, _hwnd_parent, nullptr, _hinstance, nullptr);
    if (_hwnd_callstack) {
        ListView_SetExtendedListViewStyle(
            _hwnd_callstack, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMNA col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        col.iSubItem = 0;
        col.cx = 180;
        col.pszText = const_cast<char*>("Function");
        ListView_InsertColumn(_hwnd_callstack, 0, &col);

        col.iSubItem = 1;
        col.cx = 150;
        col.pszText = const_cast<char*>("File");
        ListView_InsertColumn(_hwnd_callstack, 1, &col);

        col.iSubItem = 2;
        col.cx = 60;
        col.pszText = const_cast<char*>("Line");
        ListView_InsertColumn(_hwnd_callstack, 2, &col);
    }

    // ---- Variables text view -----------------------------------------------
    _hwnd_variables = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | ES_MULTILINE | ES_READONLY
            | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
        0, 0, 0, 0, _hwnd_parent, nullptr, _hinstance, nullptr);
    if (_hwnd_variables) {
        HFONT fixed_font
            = reinterpret_cast<HFONT>(GetStockObject(ANSI_FIXED_FONT));
        if (fixed_font)
            SendMessageW(_hwnd_variables, WM_SETFONT,
                reinterpret_cast<WPARAM>(fixed_font), TRUE);
    }

    // ---- GDI console (embedded) --------------------------------------------
    if (!nu_winconsole_is_active()) {
        nu_winconsole_init(_hinstance, SW_HIDE);
    }

    _hwnd_console = (HWND)nu_winconsole_get_hwnd();

    if (_hwnd_console) {
        // Strip top-level / overlapped styles and make it a child window so
        // it can be embedded inside the main IDE window.
        LONG_PTR style = GetWindowLongPtr(_hwnd_console, GWL_STYLE);
        style &= ~(WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU
            | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        style |= WS_CHILD | WS_CLIPSIBLINGS;
        SetWindowLongPtr(_hwnd_console, GWL_STYLE, style);

        LONG_PTR exstyle = GetWindowLongPtr(_hwnd_console, GWL_EXSTYLE);
        exstyle &= ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE
            | WS_EX_OVERLAPPEDWINDOW);
        SetWindowLongPtr(_hwnd_console, GWL_EXSTYLE, exstyle);

        SetParent(_hwnd_console, _hwnd_parent);
        // SWP_FRAMECHANGED triggers WM_NCCALCSIZE so the NC area is recomputed
        // with the new (child) styles.
        SetWindowPos(_hwnd_console, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
                | SWP_NOACTIVATE);
    }

    // Show the Output tab by default
    switch_to_tab(tab_id_t::OUTPUT);
}

/* -------------------------------------------------------------------------- */

tab_container_t::~tab_container_t()
{
    delete _infobox;
    // _hwnd_console is owned by nu_winconsole_api — do not destroy here.
}

/* -------------------------------------------------------------------------- */

void tab_container_t::insert_tab_item(
    int index, const wchar_t* text, tab_id_t id)
{
    TCITEMW tie = {};
    tie.mask = TCIF_TEXT | TCIF_PARAM;
    tie.pszText = const_cast<wchar_t*>(text);
    tie.lParam = static_cast<LPARAM>(id);
    SendMessageW(_hwnd_tab, TCM_INSERTITEMW, static_cast<WPARAM>(index),
        reinterpret_cast<LPARAM>(&tie));
}

void tab_container_t::create_tab_items()
{
    insert_tab_item(0, L"Output", tab_id_t::OUTPUT);
    insert_tab_item(1, L"Console", tab_id_t::CONSOLE);
    insert_tab_item(2, L"Call Stack", tab_id_t::CALLSTACK);
    insert_tab_item(3, L"Variables", tab_id_t::VARIABLES);
}

int tab_container_t::tab_index_of(tab_id_t tab) const
{
    const int count = TabCtrl_GetItemCount(_hwnd_tab);

    for (int index = 0; index < count; ++index) {
        TCITEMW item = {};
        item.mask = TCIF_PARAM;

        if (TabCtrl_GetItem(_hwnd_tab, index, &item)
            && static_cast<tab_id_t>(item.lParam) == tab) {
            return index;
        }
    }

    return -1;
}

tab_container_t::tab_id_t tab_container_t::tab_id_at(int sel) const
{
    if (sel < 0)
        return tab_id_t::OUTPUT;
    TCITEMW item = {};
    item.mask = TCIF_PARAM;
    TabCtrl_GetItem(_hwnd_tab, sel, &item);
    return static_cast<tab_id_t>(item.lParam);
}

/* -------------------------------------------------------------------------- */

void tab_container_t::update_call_stack(
    const std::vector<call_stack_frame_t>& frames)
{
    if (!_hwnd_callstack)
        return;

    ListView_DeleteAllItems(_hwnd_callstack);
    _callstack_frames.clear();

    // frames[0] is already the innermost (current) frame.
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        const int row = static_cast<int>(_callstack_frames.size());
        _callstack_frames.push_back(*it);

        const std::string& sname = it->func_name;
        const std::string& sfile = it->source_file;
        const std::string sline
            = it->source_line > 0 ? std::to_string(it->source_line) : "";

        LVITEMA lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = row;
        lvi.iSubItem = 0;
        lvi.pszText = sname.empty() ? const_cast<char*>("<global>")
                                    : const_cast<char*>(sname.c_str());
        ListView_InsertItem(_hwnd_callstack, &lvi);

        ListView_SetItemText(
            _hwnd_callstack, row, 1, const_cast<char*>(sfile.c_str()));
        ListView_SetItemText(
            _hwnd_callstack, row, 2, const_cast<char*>(sline.c_str()));
    }
}

void tab_container_t::clear_call_stack()
{
    if (_hwnd_callstack)
        ListView_DeleteAllItems(_hwnd_callstack);
    _callstack_frames.clear();
}

bool tab_container_t::get_callstack_selected_location(
    std::string& path, int& line) const
{
    if (!_hwnd_callstack)
        return false;
    const int sel = ListView_GetNextItem(_hwnd_callstack, -1, LVNI_SELECTED);
    if (sel < 0 || sel >= static_cast<int>(_callstack_frames.size()))
        return false;
    const auto& frame = _callstack_frames[sel];
    if (frame.source_path.empty())
        return false;
    path = frame.source_path;
    line = frame.source_line;
    return line > 0;
}

void tab_container_t::update_variables(const std::wstring& text)
{
    if (!_hwnd_variables)
        return;
    SetWindowTextW(_hwnd_variables, text.c_str());
    InvalidateRect(_hwnd_variables, nullptr, TRUE);
    UpdateWindow(_hwnd_variables);
}

void tab_container_t::clear_variables()
{
    if (_hwnd_variables)
        SetWindowTextW(_hwnd_variables, L"");
}

/* -------------------------------------------------------------------------- */

RECT tab_container_t::display_rect_in_parent() const
{
    // Get the display area inside the tab strip in TAB-CLIENT coordinates,
    // then map to PARENT-CLIENT coordinates.
    RECT rc;
    GetClientRect(_hwnd_tab, &rc);
    TabCtrl_AdjustRect(_hwnd_tab, FALSE, &rc); // remove tab-bar strip
    // MapWindowPoints converts (in-place) from _hwnd_tab's client space
    // to _hwnd_parent's client space.
    MapWindowPoints(_hwnd_tab, _hwnd_parent, reinterpret_cast<LPPOINT>(&rc), 2);
    return rc;
}

/* -------------------------------------------------------------------------- */

void tab_container_t::layout_content(const RECT& rc)
{
    int x = rc.left;
    int y = rc.top;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    if (cx <= 0 || cy <= 0)
        return;

    if (_infobox)
        _infobox->arrange(static_cast<WORD>(x), static_cast<WORD>(y),
            static_cast<WORD>(cx), static_cast<WORD>(cy));

    if (_hwnd_console && !_console_detached)
        MoveWindow(_hwnd_console, x, y, cx, cy, TRUE);

    if (_hwnd_callstack)
        MoveWindow(_hwnd_callstack, x, y, cx, cy, TRUE);

    if (_hwnd_variables)
        MoveWindow(_hwnd_variables, x, y, cx, cy, TRUE);
}

/* -------------------------------------------------------------------------- */

void tab_container_t::update_visibility()
{
    bool show_output = (_current_tab == tab_id_t::OUTPUT);
    bool show_callstack = (_current_tab == tab_id_t::CALLSTACK);
    bool show_variables = (_current_tab == tab_id_t::VARIABLES);

    if (_infobox)
        ShowWindow(_infobox->get_hwnd(), show_output ? SW_SHOW : SW_HIDE);

    if (_hwnd_callstack) {
        if (show_callstack) {
            SetWindowPos(_hwnd_callstack, HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            InvalidateRect(_hwnd_callstack, nullptr, TRUE);
        } else {
            ShowWindow(_hwnd_callstack, SW_HIDE);
        }
    }

    if (_hwnd_variables) {
        if (show_variables) {
            SetWindowPos(_hwnd_variables, HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            InvalidateRect(_hwnd_variables, nullptr, TRUE);
        } else {
            ShowWindow(_hwnd_variables, SW_HIDE);
        }
    }

    if (_hwnd_console) {
        if (_console_detached) {
            ShowWindow(_hwnd_console, SW_SHOW);
        } else {
            bool show_console = (_current_tab == tab_id_t::CONSOLE);
            ShowWindow(_hwnd_console, show_console ? SW_SHOW : SW_HIDE);
        }
    }
}

/* -------------------------------------------------------------------------- */

void tab_container_t::arrange(WORD x_pos, WORD y_pos, WORD dx, WORD dy)
{
    // 1. Resize the tab control itself.
    MoveWindow(_hwnd_tab, x_pos, y_pos, dx, dy, TRUE);

    // 2. Compute the display area in parent-client coordinates (accounts for
    //    the tab-strip height) and position child controls inside it.
    RECT rc = display_rect_in_parent();
    layout_content(rc);

    // 3. Apply visibility for the currently selected tab.
    update_visibility();

    // 4. Force repaint of active child so content tracks the new size.
    if (_current_tab == tab_id_t::OUTPUT && _infobox)
        InvalidateRect(_infobox->get_hwnd(), nullptr, TRUE);

    if (_current_tab == tab_id_t::CONSOLE && _hwnd_console
        && !_console_detached)
        InvalidateRect(_hwnd_console, nullptr, TRUE);

    if (_current_tab == tab_id_t::CALLSTACK && _hwnd_callstack)
        InvalidateRect(_hwnd_callstack, nullptr, TRUE);

    if (_current_tab == tab_id_t::VARIABLES && _hwnd_variables)
        InvalidateRect(_hwnd_variables, nullptr, TRUE);
}

/* -------------------------------------------------------------------------- */

void tab_container_t::switch_to_tab(tab_id_t tab)
{
    const int tab_index = tab_index_of(tab);
    if (tab_index < 0)
        return;

    _current_tab = tab;
    TabCtrl_SetCurSel(_hwnd_tab, tab_index);
    update_visibility();
    if (tab == tab_id_t::CONSOLE && _hwnd_console)
        nu_winconsole_refresh();
    if (tab == tab_id_t::CALLSTACK && _hwnd_callstack) {
        InvalidateRect(_hwnd_callstack, nullptr, TRUE);
        UpdateWindow(_hwnd_callstack);
    }
    if (tab == tab_id_t::VARIABLES && _hwnd_variables) {
        InvalidateRect(_hwnd_variables, nullptr, TRUE);
        UpdateWindow(_hwnd_variables);
    }
}

/* -------------------------------------------------------------------------- */

void tab_container_t::on_notify(LPNMHDR pnmhdr)
{
    if (pnmhdr->hwndFrom == _hwnd_tab && pnmhdr->code == TCN_SELCHANGE) {
        int sel = TabCtrl_GetCurSel(_hwnd_tab);
        if (sel >= 0)
            switch_to_tab(tab_id_at(sel));
    }
}

/* -------------------------------------------------------------------------- */

void tab_container_t::toggle_console_detach()
{
    if (!_hwnd_console)
        return;

    if (!_console_detached) {
        // ---- Detach: make console a top-level window -----------------------
        _console_detached = true;

        // Remove the Console tab from the tab control while detached.
        const int console_tab_index = tab_index_of(tab_id_t::CONSOLE);
        if (console_tab_index >= 0)
            TabCtrl_DeleteItem(_hwnd_tab, console_tab_index);
        _current_tab = tab_id_t::OUTPUT;
        const int output_tab_index = tab_index_of(tab_id_t::OUTPUT);
        if (output_tab_index >= 0)
            TabCtrl_SetCurSel(_hwnd_tab, output_tab_index);

        ShowWindow(_hwnd_console, SW_HIDE);

        LONG_PTR style = GetWindowLongPtr(_hwnd_console, GWL_STYLE);
        style &= ~(WS_CHILD | WS_CLIPSIBLINGS);
        style |= WS_OVERLAPPEDWINDOW | WS_VSCROLL;
        SetWindowLongPtr(_hwnd_console, GWL_STYLE, style);

        SetParent(_hwnd_console, nullptr);
        // Avoid synchronous client-area repaint while the interpreter may hold
        // the GDI surface mutex (see ConsoleWindow::on_paint try_lock).
        SetWindowPos(_hwnd_console, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOREDRAW);

        RECT parent_rc;
        GetWindowRect(_hwnd_parent, &parent_rc);
        int w = 640, h = 400;
        int x = parent_rc.right + 10;
        int y = parent_rc.top;
        MoveWindow(_hwnd_console, x, y, w, h, FALSE);

        // Update infobox to fill the now-solo Output tab area.
        RECT rc = display_rect_in_parent();
        layout_content(rc);
        update_visibility();

        ShowWindow(_hwnd_console, SW_SHOW);
        SetForegroundWindow(_hwnd_console);
        nu_winconsole_refresh();
    } else {
        // ---- Re-attach: embed console back as a child ----------------------
        _console_detached = false;

        ShowWindow(_hwnd_console, SW_HIDE);

        LONG_PTR style = GetWindowLongPtr(_hwnd_console, GWL_STYLE);
        style &= ~(WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU
            | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VSCROLL);
        style |= WS_CHILD | WS_CLIPSIBLINGS;
        SetWindowLongPtr(_hwnd_console, GWL_STYLE, style);

        LONG_PTR exstyle = GetWindowLongPtr(_hwnd_console, GWL_EXSTYLE);
        exstyle &= ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE
            | WS_EX_OVERLAPPEDWINDOW);
        SetWindowLongPtr(_hwnd_console, GWL_EXSTYLE, exstyle);

        SetParent(_hwnd_console, _hwnd_parent);
        SetWindowPos(_hwnd_console, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
                | SWP_NOACTIVATE);

        // Re-insert the Console tab at its original position (index 1).
        insert_tab_item(1, L"Console", tab_id_t::CONSOLE);

        RECT rc = display_rect_in_parent();
        layout_content(rc);
        switch_to_tab(tab_id_t::CONSOLE);
    }
}

/* -------------------------------------------------------------------------- */
