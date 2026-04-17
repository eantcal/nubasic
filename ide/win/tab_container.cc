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
    // ---- Tab control -------------------------------------------------------
    _hwnd_tab = CreateWindowW(WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_BOTTOM, 0, 0, 0, 0,
        _hwnd_parent, nullptr, _hinstance, nullptr);
    assert(_hwnd_tab);

    create_tab_items();

    // ---- Output infobox (rich-edit) ----------------------------------------
    _infobox = new txtinfobox_t(_hwnd_parent, _hinstance);

    // ---- Call Stack ListView -----------------------------------------------
    _hwnd_callstack = CreateWindowExW(0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_CLIPSIBLINGS | LVS_REPORT | LVS_NOSORTHEADER
            | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, _hwnd_parent, nullptr, _hinstance, nullptr);
    if (_hwnd_callstack) {
        ListView_SetExtendedListViewStyle(
            _hwnd_callstack, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        LVCOLUMNW col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH;
        wchar_t fn[] = L"Function";
        col.pszText = fn;
        col.cx = 240;
        SendMessageW(_hwnd_callstack, LVM_INSERTCOLUMNW, 0,
            reinterpret_cast<LPARAM>(&col));
        wchar_t ln[] = L"Called from line";
        col.pszText = ln;
        col.cx = 120;
        SendMessageW(_hwnd_callstack, LVM_INSERTCOLUMNW, 1,
            reinterpret_cast<LPARAM>(&col));
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
    const std::vector<std::pair<std::string, int>>& frames)
{
    if (!_hwnd_callstack)
        return;

    ListView_DeleteAllItems(_hwnd_callstack);

    int row = 0;
    for (auto it = frames.rbegin(); it != frames.rend(); ++it, ++row) {
        LVITEMW lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = row;

        std::wstring name(it->first.begin(), it->first.end());
        lvi.pszText = name.data();
        SendMessageW(_hwnd_callstack, LVM_INSERTITEMW, 0,
            reinterpret_cast<LPARAM>(&lvi));

        std::wstring line_str = std::to_wstring(it->second);
        LVITEMW lvi_sub = {};
        lvi_sub.iSubItem = 1;
        lvi_sub.pszText = line_str.data();
        SendMessageW(_hwnd_callstack, LVM_SETITEMTEXTW,
            static_cast<WPARAM>(row), reinterpret_cast<LPARAM>(&lvi_sub));
    }
}

void tab_container_t::clear_call_stack()
{
    if (_hwnd_callstack)
        ListView_DeleteAllItems(_hwnd_callstack);
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
}

/* -------------------------------------------------------------------------- */

void tab_container_t::update_visibility()
{
    bool show_output = (_current_tab == tab_id_t::OUTPUT);
    bool show_callstack = (_current_tab == tab_id_t::CALLSTACK);

    if (_infobox)
        ShowWindow(_infobox->get_hwnd(), show_output ? SW_SHOW : SW_HIDE);

    if (_hwnd_callstack)
        ShowWindow(_hwnd_callstack, show_callstack ? SW_SHOW : SW_HIDE);

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
}

/* -------------------------------------------------------------------------- */

void tab_container_t::switch_to_tab(tab_id_t tab)
{
    _current_tab = tab;
    TabCtrl_SetCurSel(_hwnd_tab, static_cast<int>(tab));
    update_visibility();
    if (tab == tab_id_t::CONSOLE && _hwnd_console)
        nu_winconsole_refresh();
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
        TabCtrl_DeleteItem(_hwnd_tab, static_cast<int>(tab_id_t::CONSOLE));
        _current_tab = tab_id_t::OUTPUT;
        TabCtrl_SetCurSel(_hwnd_tab, static_cast<int>(tab_id_t::OUTPUT));

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
        insert_tab_item(
            static_cast<int>(tab_id_t::CONSOLE), L"Console", tab_id_t::CONSOLE);

        RECT rc = display_rect_in_parent();
        layout_content(rc);
        switch_to_tab(tab_id_t::CONSOLE);
    }
}

/* -------------------------------------------------------------------------- */
