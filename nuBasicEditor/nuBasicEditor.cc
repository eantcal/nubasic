/*
 *  This file is part of nuBASIC
 *
 *  nuBASIC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  nuBASIC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with nuBASIC; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
 *
 *  Author: Antonino Calderone <acaldmail@gmail.com>
 *
 */


/* -------------------------------------------------------------------------- */

#include "stdafx.h"


#include "nu_about.h"
#include "nu_builtin_help.h"
#include "nu_builtin_help.h"
#include "nu_exception.h"
#include "nu_interpreter.h"
#include "nu_reserved_keywords.h"

#include "textinfobox.h"
#include "toolbar.h"

#include <regex>
#include <set>
#include <thread>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

namespace editor {

    /* --------------------------------------------------------------------------
     */

    const char application_name[] = EDITOR_RESOURCE_NAME;
    const char class_name[] = EDITOR_RESOURCE_NAME "Window";

    const COLORREF black = RGB(0, 0, 0);
    const COLORREF white = RGB(0xff, 0xff, 0xff);
    const COLORREF red = RGB(0xff, 0, 0);
    const COLORREF yellow = RGB(0xff, 0xff, 0xe0);
    const COLORREF offWhite = RGB(0xff, 0xfb, 0xF0);
    const COLORREF darkGreen = RGB(0, 0x80, 0);
    const COLORREF darkBlue = RGB(0, 0, 0x80);


    /* --------------------------------------------------------------------------
     */

    const int toolbar_n_of_bmps = 11;
    const int toolbar_btn_state = TBSTATE_ENABLED;
    const int toolbar_btn_style = BTNS_BUTTON | TBSTATE_ELLIPSES;


    TBBUTTON toolbar_buttons[]
        = { { 0, 0, TBSTATE_ENABLED, BTNS_SEP, { 0 }, NULL, NULL },

            { 0, IDM_FILE_NEW, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "New" },
            { 1, IDM_FILE_OPEN, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Open" },
            { 2, IDM_FILE_SAVE, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Save" },

            { 0, 0, TBSTATE_ENABLED, BTNS_SEP, { 0 }, NULL, NULL },

            { 3, IDM_DEBUG_START, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Run" },
            { 4, IDM_DEBUG_STEP, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Step" },
            { 5, IDM_DEBUG_CONT, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Continue" },
            { 6, IDM_DEBUG_TOGGLEBRK, toolbar_btn_state, toolbar_btn_style,
                { 0 }, NULL, (INT_PTR) "Breakpoint" },
            { 7, IDM_INTERPRETER_BUILD, toolbar_btn_state, toolbar_btn_style,
                { 0 }, NULL, (INT_PTR) "Build" },
            { 8, IDM_DEBUG_EVALSEL, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Evaluate" },

            { 0, 0, TBSTATE_ENABLED, BTNS_SEP, { 0 }, NULL, NULL },

            { 9, IDM_SEARCH_FIND, toolbar_btn_state, toolbar_btn_style, { 0 },
                NULL, (INT_PTR) "Find" },

            { 0, 0, TBSTATE_ENABLED, BTNS_SEP, { 0 }, NULL, NULL },

            { 10, IDM_DEBUG_TOPMOST, toolbar_btn_state, toolbar_btn_style,
                { 0 }, NULL, (INT_PTR) "Dbg Top" },
            { 11, IDM_DEBUG_NOTOPMOST, toolbar_btn_state, toolbar_btn_style,
                { 0 }, NULL, (INT_PTR) "Dbg No-Top" }

          };

    const int toolbar_n_of_buttons = sizeof(toolbar_buttons) / sizeof(TBBUTTON);


    /* --------------------------------------------------------------------------
     */

    struct autocompl_t {
        std::string data;

        autocompl_t()
        {
            for (auto& token : nu::reserved_keywords_t::list) {
                data += token;
                data += " ";
            }
        }
    };


    static autocompl_t autocomplete_list;


    /* --------------------------------------------------------------------------
     */

} // namespace editor


/* -------------------------------------------------------------------------- */

//! nuBasic Editor application implementation
//
class editor_t {
    friend LRESULT CALLBACK HSplitterWndProc(HWND, WORD, WORD, LONG);
    nu::interpreter_t _interpreter;

public:
    enum {
        SPLIT_BAR_WIDTH = 8,
        SPLIT_BAR_HEIGHT = 8,

        SEARCH_BOX_X = 10,
        SEARCH_BOX_Y = 10,
        SEARCH_BOX_DX = 100,
        SEARCH_BOX_DY = 30
    };

    enum class marker_t {
        BOOKMARK = 0,
        BREAKPOINT = 1,
        PROGCOUNTER = 2,
        LINESELECTION = 4,
    };

    enum { TIMER_EVAL_SELECTION, TIMER_CTX_HELP };

    nu::interpreter_t& interpreter() { return _interpreter; }

    enum {
        LINENUM_WIDTH = 5,
        DEF_TABWIDTH = 3,
        DEF_INDENT = DEF_TABWIDTH,
        DEF_MARGIN_WIDTH = 16,
        DEF_CARETPERIOD = 400 // ms
    };


    /**
    * editor_t ctor
    */
    editor_t();


    /**
    * Set main window handle
    */
    void set_hwnd(HWND hWnd) noexcept { _hwnd_main = hWnd; }


    /**
    * Get main window handle
    */
    HWND get_main_hwnd() const noexcept { return _hwnd_main; }


    /**
    * Get editor window handle
    */
    HWND get_editor_hwnd() const noexcept { return _hwnd_editor; }


    /**
     * Set editor window handle
     */
    void set_editor_hwnd(HWND hwnd) noexcept { _hwnd_editor = hwnd; }


    /**
     * Set splitter position
     */
    void set_splitbar_pos(DWORD wDX, DWORD oDY)
    {
        MoveWindow(_h_splitter, 0, oDY, wDX, SPLIT_BAR_HEIGHT, TRUE);
    }


    /**
    * Set splitter position
    */
    void set_infobox_pos(DWORD x, DWORD y, DWORD dx, DWORD dy)
    {
        MoveWindow(_h_infobox, x, y, dx, dy, TRUE);
    }


    /**
     * Create splitter control
     */
    void create_splitter(HWND hWnd);


    /**
     * Create search controls
     */
    void create_search_replace_cntrls(HWND hWnd);


    /**
     * Append a string to info box
     */
    void add_info(std::string msg, DWORD message_style);


    /**
     * Clear info box content to clipboard
     */
    void copy_info_to_clipboard();


    /**
     * Clear info box
     */
    void clear_info();


    /**
    * Hide info box
    */
    void hide_info() { resize_info(10); }


    /**
    * Resize info box
    */
    void resize_info(int editor_tenth = 9);


    /**
    * Handle WM_DROPFILES message received from the application window
    * which has registered as a recipient of dropped files
    */
    void on_drop_files(HDROP hDropInfo);


    /**
    * Set replace msg
    */
    void set_find_replace_msg(UINT msg) noexcept { _find_replace_msg = msg; }


    /**
    * Get replace msg
    */
    UINT get_find_replace_msg() const noexcept { return _find_replace_msg; }


    /**
    * Get instance handle
    */
    HINSTANCE get_instance_handle() const noexcept { return _hInstance; }


    /**
    * Set instance handle
    */
    void set_instance_handle(HINSTANCE hInst) noexcept
    {
        _hInstance = hInst;
    }


    /**
    * Export a reference to the current dialog window handle
    */
    HWND& current_dialog() noexcept { return _current_dialog; }


    /**
    * Set the reference to the current dialog window handle
    */
    void set_current_dialog(HWND hwnd) { _current_dialog = hwnd; }


    /**
    * Export a reference to the "Go to ..." line
    */
    int& goto_line() noexcept { return _goto_line; }


    /**
    * Send a command to the editor control
    */
    LRESULT send_command(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) const
    {
        return ::SendMessage(_hwnd_editor, Msg, wParam, lParam);
    }

    /**
    * Get editor text within the range [start, end]
    */
    void get_text_range(int start, int end, char* text) const;


    /**
    * Set main window title to current file name
    */
    void set_title();


    /**
    * New document
    */
    void set_new_document(bool clear_title = true);


    /**
    * Show open document dialog
    */
    void open_document();


    /**
    * Open document filename
    */
    void open_document_file(const char* fileName);


    /**
    * Save editing document
    */
    void save_document();


    /**
    * Show "save as..." dialog
    */
    void save_document_as();


    /**
    * Save editing document with fileName
    */
    void save_file(const std::string& filename);


    /**
    * Check and save open document if needed
    */
    int save_if_unsure();


    /**
    * Dispatch menu and accelerator commands
    */
    void exec_command(int id);


    /**
    * Enable / disable menu item accordlying current editor state
    */
    void check_menus();


    /**
    * Process editor notifications
    */
    void notify(SCNotification* notification);


    /**
    * Set editor item style
    */
    void set_item_style(int style, COLORREF fore, COLORREF back = editor::white,
        int size = -1, const char* face = 0);

    /**
    * Get line count
    */
    int get_line_count() const noexcept
    {
        return int((send_command(SCI_GETLINECOUNT, 0, 0)));
    }


    /**
    * Toggle the display of the folding margin
    */
    void set_folding_margin(bool enable) noexcept
    {
        send_command(SCI_SETMARGINWIDTHN, 2, enable ? DEF_MARGIN_WIDTH : 0);
    }


    /**
    * Calculate the width for line numbers
    * \return number of pixels for the margin width of margin (0)
    */
    int get_line_num_width() const noexcept
    {
        return int(LINENUM_WIDTH
            * send_command(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)("9")));
    }


    /**
    *  Set the display of line numbers on or off.
    * \param enable if we shuld display line numbers
    */
    void set_numbers_margin(bool enable)
    {
        send_command(
            SCI_SETMARGINWIDTHN, 0, enable ? get_line_num_width() + 4 : 0);
    }


    /**
    * Toggle the display of the selection bookmark margin
    */
    void set_selection_margin(bool enable)
    {
        send_command(SCI_SETMARGINWIDTHN, 1, enable ? DEF_MARGIN_WIDTH : 0);
    }


    /**
    * Initilize the editor
    */
    void init_editor(const std::string& fontname, int height);


    /**
    * Refresh editor
    */
    void refresh();


    /**
    * Update UI and do brace matching
    */
    void update_ui();


    /**
    * Add a bookmark if not already present at given line
    * Otherwise remove it
    * \param line where to add/remove bookmark - lines start at 1
    */
    void toggle_bookmark(long line) noexcept
    {
        // just one of following function will be effect !
        if (!add_bookmark(line))
            remove_bookmark(line);
    }


    /**
    * Add a bookmark at given line
    * \param line where to add bookmark - lines start at 1
    */
    bool add_bookmark(long line) noexcept
    {
        if (!has_bookmark(line)) {
            const auto m = int(marker_t::BOOKMARK);

            send_command(SCI_MARKERDEFINE, m, SC_MARK_BOOKMARK);
            send_command(SCI_MARKERSETFORE, m, RGB(0, 0, 0));
            send_command(SCI_MARKERSETBACK, m, RGB(0, 255, 0));

            send_command(SCI_MARKERADD, line - 1, m);
            return true;
        }

        return false;
    }


    /**
    * Remove the program counter marker
    */
    void remove_prog_cnt_marker()
    {
        send_command(SCI_MARKERDELETEALL, int(marker_t::PROGCOUNTER), 0);
        send_command(SCI_MARKERDELETEALL, int(marker_t::LINESELECTION), 0);
        send_command(SCI_LINESCROLLDOWN, 0, 0);
        send_command(SCI_LINESCROLLUP, 0, 0);
    }


    /**
    * Show execution point
    */
    bool show_execution_point(int line) noexcept;


    /**
    * Show error line
    */
    bool show_error_line(int line) noexcept;


    /**
    * Remove a bookmark at given line
    * \param line where to delete bookmark - lines start at 1
    */
    bool remove_bookmark(long line) noexcept
    {
        if (has_bookmark(line)) {
            send_command(SCI_MARKERDELETE, line - 1, 0);
            return true;
        }

        return false;
    }


    /**
    * Add a breakpoint at given line
    * \param line where to add breakpoint - lines start at 1
    */
    bool add_breakpoint(long line) noexcept
    {
        const auto m = int(marker_t::BREAKPOINT);
        send_command(SCI_MARKERDEFINE, m, SC_MARK_CIRCLE);
        send_command(SCI_MARKERSETFORE, m, RGB(255, 255, 255));
        send_command(SCI_MARKERSETBACK, m, RGB(255, 0, 0));

        send_command(SCI_MARKERADD, line - 1, m);

        return true;
    }


    /**
    * Add a breakpoint at given line
    * \param line where to add breakpoint - lines start at 1
    */
    bool toggle_breakpoint(long line) noexcept
    {
        if (!remove_breakpoint(line))
            add_breakpoint(line);

        return true;
    }


    /**
    * Remove a breakpoint at given line
    * \param line where to delete bookmark - lines start at 1
    */
    bool remove_breakpoint(long line) noexcept
    {
        if (has_breakpoint(line)) {
            send_command(SCI_MARKERDELETE, line - 1, 1);
            return true;
        }

        return false;
    }


    /**
    * Remove all bookmarks
    */
    void remove_all_bookmarks() noexcept
    {
        send_command(SCI_MARKERDELETEALL, int(marker_t::BOOKMARK), 0);
    }


    /**
    * Remove all breakpoints
    */
    void remove_all_breakpoints() noexcept
    {
        send_command(SCI_MARKERDELETEALL, int(marker_t::BREAKPOINT), 0);
    }


    /**
    * Reset all breakpoints
    */
    void reset_all_breakpoints() noexcept
    {
        auto linecount = get_line_count();

        interpreter().exec_command("clrbrk");

        for (int i = 0; i < linecount; ++i)
            if (has_breakpoint(i + 1))
                interpreter().exec_command("break " + std::to_string(i + 1));
    }


    /**
    * Check if given line has a bookmark
    * \param line where to add bookmark - lines start at 1
    * \return true if given line has a bookmark - otherwise false
    */
    bool has_bookmark(long line) const noexcept
    {
        const auto m = int(marker_t::BOOKMARK) + 1;
        return ((send_command(SCI_MARKERGET, line - 1, 0) & m) == m);
    }

    /**
    * Check if given line has a breakpoint
    * \param line where to add breakpoint - lines start at 1
    * \return true if given line has a breakpoint - otherwise false
    */
    bool has_breakpoint(long line) const noexcept
    {
        const auto m = int(marker_t::BREAKPOINT) + 1;
        return ((send_command(SCI_MARKERGET, line - 1, 0) & m) == m);
    }


    /**
    * Find next bookmark from current line
    */
    void find_next_bookmark();


    /**
    * Find previous bookmark from current line
    */
    void find_prev_bookmark();


    /**
    * Goto given character position
    * \param pos new character position
    */
    void go_to_pos(long pos) noexcept { send_command(SCI_GOTOPOS, pos, 0); }


    /**
    * Get the current line number - this the with the caret in it
    * \return line number with the caret in it - starts with 1
    */
    LRESULT get_current_line() const noexcept;


    /**
    * Get the current column number = position of the caret within the line.
    * This return value my be affected by the TAB setting! Starts with 1
    * \return current column number
    */
    LRESULT get_current_colum() const noexcept;


    /**
    * Return the current character position within the file.
    * \return current character position
    */
    LRESULT get_current_position() const noexcept;


    /**
    * Return the current style at the caret
    * \return the current style index
    */
    LRESULT get_current_style() const noexcept;


    /**
    * Return the current folding level at the caret line
    * \return the current folding level
    */
    int get_fold_level() const noexcept;


    /**
    * Set the fontname
    */
    void set_font(int style, const std::string& name) noexcept
    {
        send_command(
            SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(name.c_str()));
    }


    /**
    * Set the font height in points
    */
    void set_font_height(int style, int height) noexcept
    {
        send_command(SCI_STYLESETSIZE, style, static_cast<LPARAM>(height));
    }


    /**
    * Set the foreground color
    */
    void set_fg(int style, COLORREF crForeground) noexcept
    {
        send_command(
            SCI_STYLESETFORE, style, static_cast<LPARAM>(crForeground));
    }


    /**
    * Set the backgroundcolor
    */
    void set_bg(int style, COLORREF crBackground) noexcept
    {
        send_command(
            SCI_STYLESETBACK, style, static_cast<LPARAM>(crBackground));
    }


    /**
    * Set given style to bold
    */
    void set_font_bold(int style, bool enable) noexcept
    {
        send_command(SCI_STYLESETBOLD, style, static_cast<LPARAM>(enable));
    }


    /**
    * Set given style to italic
    */
    void set_font_italic(int style, bool enable) noexcept
    {
        send_command(SCI_STYLESETITALIC, style, static_cast<LPARAM>(enable));
    }


    /**
    * Set given style to underline
    */
    void set_font_underline(int style, bool enable) noexcept
    {
        send_command(SCI_STYLESETUNDERLINE, style, static_cast<LPARAM>(enable));
    }


    /**
    * Get true if overstrike is enabled
    */
    bool get_overstrike() const noexcept
    {
        return send_command(SCI_GETOVERTYPE, 0, 0) != 0;
    }


    /**
    * Set overstrike state
    */
    void nu::editor_t::set_overstrike(bool enable) noexcept
    {
        send_command(SCI_SETOVERTYPE, enable ? TRUE : FALSE, 0);
    }

    /**
    * Goto givven line position
    * \param line new line - lines start at 1
    */
    void go_to_line(long line) { send_command(SCI_GOTOLINE, line - 1, 0); }


    /**
    * Search forward for a given string and select it if found.
    * You may use regular expressions on the text.
    * \param text to search
    * \return TRUE if text is ffound else FALSE
    */
    bool search_forward(LPSTR szText);


    /**
    * Search backward for a given string and select it if found.
    * You may use regular expressions on the text.
    * \param szText text to search
    * \return TRUE if text is ffound else FALSE
    */
    bool search_backward(LPSTR szText);


    /**
    * Replace a text with a new text. You can use regular
    * expression with tagging on the replacement text.
    * \param szText new text
    */
    void replace_searching_text(PCSTR szText);


    /**
    * Get start of selection as character position
    * \return character position of selection begin
    *  otherwise -1 on error
    */
    LRESULT get_selection_begin() const noexcept
    {
        return send_command(SCI_GETSELECTIONSTART, 0, 0);
    }


    /**
    * Get end of selection as character position
    * \return character position of selection end
    *  otherwise -1 on error
    */
    LRESULT get_selection_end() const noexcept
    {
        return send_command(SCI_GETSELECTIONEND, 0, 0);
    }


    /**
    * Get selected string
    * \return string with currentliy selected text
    */
    std::string get_selection();


    /**
    * Replace a text in a selection or in the complete file multiple times
    * \return number of replacements
    */
    int replace_all(LPCSTR szFind, LPCSTR szReplace, BOOL bUseSelection);


    /*
    * Called from WinMain to set application command line
    * \param cmdLine passed to WinMain startup function
    */
    void set_command_line(LPCSTR cmdLine) { _command_line = cmdLine; }


    /**
    * Returns program command line (set with set_command_line)
    * \return command line
    */
    const std::string& get_command_line() const { return _command_line; }


    /**
       * Returns scintilla search flags
       * \return flags
       */
    int get_search_flags() const
    {
        return int(send_command(SCI_GETSEARCHFLAGS, 0, 0));
    }


    /**
    * Set up search flags to scintilla
    * \return flags
    */
    void set_search_flags(int flags)
    {
        _search_flags = flags;
        send_command(SCI_SETSEARCHFLAGS, _search_flags, 0);
    }


    /**
    * Invert search direction flg
    */
    void invert_search_direction() noexcept
    {
        _invert_search_direction = !_invert_search_direction;
    }


    /**
    * Get search direction flg
    */
    bool get_search_direction() const noexcept
    {
        return _invert_search_direction;
    }


    /**
    * Rebuild code and check code syntax
    */
    bool rebuild_code(bool show_msg_error) noexcept;


    /**
    * Remove functions reference menu
    */
    void remove_funcs_menu() noexcept
    {
        HMENU hmenu = GetMenu(get_main_hwnd());

        if (_func_submenu)
            DeleteMenu(hmenu, (UINT)1, MF_BYCOMMAND);
    }

    /**
    * Create functions reference menu
    */
    void create_funcs_menu() noexcept;


    /**
    * Resolve function line using id of "functions" menu
    */
    int resolve_funclinenum_from_id(int id) const noexcept
    {
        const auto i = _func_map.find(id);

        if (i == _func_map.end())
            return -1;

        return i->second;
    }


    /**
    * Show splash
    */
    void show_splash();


    /**
    * Create a console if needed
    */
    void alloc_console();


    enum class dbg_flg_t {
        NORMAL_EXECUTION,
        CONTINUE_EXECUTION = 1,
        SINGLE_STEP_EXECUTION = 2
    };

    /**
    * Start program debugging
    */
    void start_debugging(dbg_flg_t flg = dbg_flg_t::NORMAL_EXECUTION);


    /**
    * Start program debugging
    */
    void continue_debugging()
    {
        start_debugging(dbg_flg_t::CONTINUE_EXECUTION);
    }

    /**
    * Continue one statement
    */
    void singlestep_debugging()
    {
        start_debugging(dbg_flg_t::SINGLE_STEP_EXECUTION);
    }


    /**
    * Evaluate selection
    */
    void eval_sel();


    /**
     * Show on-line Help
     */
    void show_ctx_help();


    /**
    * Show running-mode dialog box
    */
    void show_running_dialog();


    /**
    * Send a command to interpreter
    */
    bool exec_interpreter_cmd(const std::string& cmd, bool bgmode);


    /**
    * Ask the interpreter to evaluate an expression
    */
    bool evaluate_expression(const std::string& expression);


    /*
    * Return true if document has been modified but not saved
    */
    bool is_dirty() const noexcept { return _is_dirty; }

protected:
    using func_map_t = std::map<int, int>;
    func_map_t _func_map;
    HMENU _func_submenu = (HMENU) nullptr;

    HINSTANCE _hInstance;
    HWND _current_dialog;
    HWND _hwnd_main;
    HWND _hwnd_editor;

    HWND _h_splitter;
    HWND _h_infobox;

    bool _is_dirty = false;
    bool _need_build = true;

    int _goto_line = 0;
    std::string _command_line;

    ///! Open/Save dialog file filter
    const char* filter = EDITOR_FILE_FILTER;

    //! Set dirty flag
    void set_dirty_flag()
    {
        _is_dirty = true;
        _need_build = true;
    }

    /**
    * Called from notification handler. Set default folding
    *
    * \param margin maring we want to handle
    * \param pos character position where user clicked margin
    */
    void set_def_folding(int margin, long pos);

    /**
    * Enable/Disable a menu item
    */
    void enable_menu_item(int id, bool enable);

    //! build the basic line
    bool build_basic_line(
        const std::string& line, int line_num, bool dump_err_msg);

    // Search and replace stuff
    UINT _find_replace_msg = 0;
    std::string _full_path_str;
    FINDREPLACE _find_replace_data;
    int _search_flags = 0;
    bool _invert_search_direction = false;

    enum { FIND_STR_LEN = 1024 };

    char _find_str[FIND_STR_LEN];
    char _replace_str[FIND_STR_LEN];

    LOGFONT _logfont;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

class winMsgProc {
private:
    nu::editor_t& _editor;

public:
    winMsgProc(nu::editor_t& editor)
        : _editor(editor)
    {
    }


    int processWinMsg(HACCEL hAccTable,
        std::function<bool(void*)> cbk_f = nullptr, void* cbk_data = nullptr)
    {
        bool going = true;
        MSG msg;
        msg.wParam = 0;

        while (going) {
            if (cbk_f)
                if (cbk_f(cbk_data))
                    break;

            going = GetMessage(&msg, NULL, 0, 0) != 0;

            if (_editor.current_dialog() && going) {
                if (!IsDialogMessage(_editor.current_dialog(), &msg)) {
                    if (TranslateAccelerator(msg.hwnd, hAccTable, &msg) == 0) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            } else if (going) {
                if (TranslateAccelerator(
                        _editor.get_main_hwnd(), hAccTable, &msg)
                    == 0) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        return (int)msg.wParam;
    }
};


/* -------------------------------------------------------------------------- */

static nu::editor_t g_editor;
static HACCEL g_hAccTable = nullptr;
static winMsgProc g_winMsgProc(g_editor);
static toolbar_t* g_toolbar = nullptr;
static txtinfobox_t* g_info = nullptr;


/* -------------------------------------------------------------------------- */

void nu::editor_t::create_splitter(HWND hWnd)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)HSplitterWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = get_instance_handle();
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_SIZENS);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "HSPLITTER_WND_CLASS";
    wcex.hIconSm = NULL;

    RegisterClassEx(&wcex);

    _h_splitter = CreateWindow(TEXT("HSPLITTER_WND_CLASS"), NULL,
        WS_CHILD | WS_VISIBLE, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, hWnd,
        (HMENU)IDD_HSPLITTER, // Control identifier
        get_instance_handle(), NULL);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::create_search_replace_cntrls(HWND hWnd)
{
    auto ver = interpreter().version();

    _h_infobox = CreateWindowEx(
        WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, // make rich edit control appear
        "RICHEDIT", // class name of rich edit control
        ver.c_str(), // text of rich edit control
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_SAVESEL | ES_READONLY
            | WS_HSCROLL | WS_VSCROLL,

        0, 0, // initially create 0 size,
        0, 0, // main window's WM_SIZE handler will resize
        hWnd, // use main parent
        (HMENU)0,
        get_instance_handle(), // this app instance owns this window
        NULL);

    HANDLE hFont = NULL;
    LOGFONT lFont;

    lFont.lfHeight = 14;
    lFont.lfWidth = 0;
    lFont.lfEscapement = 0;
    lFont.lfOrientation = 0;
    lFont.lfWeight = 0;
    lFont.lfItalic = 0;
    lFont.lfUnderline = 0;
    lFont.lfStrikeOut = 0;
    lFont.lfCharSet = ANSI_CHARSET;
    lFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lFont.lfQuality = ANTIALIASED_QUALITY;
    lFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    strcpy(lFont.lfFaceName, "Consolas");
    hFont = CreateFontIndirect(&lFont);
    SendMessage(_h_infobox, WM_SETFONT, (WPARAM)hFont, (DWORD)TRUE);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::copy_info_to_clipboard()
{
    if (IDYES
        == MessageBox(get_main_hwnd(),
               "Do you want to save status information to the clipboard ?",
               "Clear message window", MB_ICONQUESTION | MB_YESNO)) {
        SendMessage(_h_infobox, EM_SETSEL, 0, -1);
        SendMessage(_h_infobox, WM_COPY, 0, 0);
        MessageBox(get_main_hwnd(), "Logs copied to the clipboard !",
            "Operation completed", MB_OK | MB_ICONINFORMATION);
    }
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::clear_info()
{
    if (IDYES == MessageBox(get_main_hwnd(), "Are you sure ?",
                     "Clear message window", MB_ICONQUESTION | MB_YESNO)) {
        copy_info_to_clipboard();

        SendMessage(_h_infobox, EM_SETSEL, 0, -1);
        SendMessage(_h_infobox, EM_REPLACESEL, 0, 0);
    }
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::on_drop_files(HDROP hDropInfo)
{
    // A file is being dropped.
    int iFiles;
    char lpszFile[MAX_PATH + 1] = { 0 };

    // Get the number of files.
    iFiles = DragQueryFile(hDropInfo, (DWORD)(-1), (LPSTR)NULL, 0);

    if (iFiles != 1) {
        MessageBox(get_main_hwnd(), "One file at a time, please.", NULL, MB_OK);
    } else {
        DragQueryFile(hDropInfo, 0, lpszFile, sizeof(lpszFile));
        std::string fileName;

        if (lpszFile) {
            open_document_file(lpszFile);
        }
    }

    DragFinish(hDropInfo);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::resize_info(int editor_tenth)
{
    RECT toolbar_rect = { 0 };
    if (g_toolbar) {
        g_toolbar->on_resize();
        g_toolbar->get_rect(toolbar_rect);
    }

    const int dy = toolbar_rect.bottom - toolbar_rect.top;
    const int dx = toolbar_rect.right - toolbar_rect.left;

    RECT rc;
    ::GetClientRect(get_main_hwnd(), &rc);

    if (g_info) {
        g_info->arrange(dx - 100, 5, 90, dy - 20);
    }

    LONG editor_size = 0;

    editor_size = editor_tenth * ((rc.bottom - rc.top - dy) / 10)
        - nu::editor_t::SPLIT_BAR_HEIGHT;

    ::SetWindowPos(g_editor.get_editor_hwnd(), 0, rc.left, rc.top + dy,
        rc.right - rc.left, editor_size, 0);

    set_splitbar_pos(rc.right - rc.left, rc.top + dy + editor_size);

    const auto y_bottom
        = rc.top + dy + editor_size + nu::editor_t::SPLIT_BAR_HEIGHT;

    set_infobox_pos(
        0, y_bottom, rc.right - rc.left, rc.bottom - rc.top - y_bottom);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::add_info(std::string msg, DWORD message_style)
{
    CHARFORMAT char_format = { 0 };

    char_format.cbSize = sizeof(char_format);
    char_format.dwMask = CFM_BOLD | CFM_ITALIC | CFE_UNDERLINE;

    SendMessage(_h_infobox, EM_GETCHARFORMAT, 0, (LPARAM)&char_format);

    SYSTEMTIME st;
    GetLocalTime(&st);

    char szDateTime[256];
    sprintf(szDateTime, "\n%02i-%02i-%i %02i:%02i:%02i\n", st.wDay, st.wMonth,
        st.wYear, st.wHour, st.wMinute, st.wSecond);

    SendMessage(_h_infobox, EM_SCROLL, (LPARAM)SB_TOP, 0);

    SendMessage(_h_infobox, EM_SETSEL, 0, 0);

    SendMessage(
        _h_infobox, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&char_format);

    char_format.dwEffects = message_style;

    SendMessage(_h_infobox, EM_REPLACESEL, 0, (LPARAM)szDateTime);

    SendMessage(
        _h_infobox, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&char_format);

    msg += "\n";

    SendMessage(_h_infobox, EM_REPLACESEL, 0, (LPARAM)msg.c_str());
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::build_basic_line(
    const std::string& line, int line_num, bool dump_err_msg)
{
    try {
        // Ignore first line if it begins with #!
        if (line_num == 1 && line.size() > 2 && line[0] == '#'
            && line[1] == '!')
            return true;

        if (!g_editor.interpreter().update_program(line, line_num)) {
            std::string msg = "Syntax Error at line ";
            msg += line_num ? std::to_string(line_num) : "\r\n";
            msg += line_num ? "" : line;

            show_error_line(line_num);

            if (dump_err_msg) {
                g_editor.add_info("\n" + msg + "\n", CFM_BOLD | CFM_ITALIC);
                MessageBox(
                    get_main_hwnd(), msg.c_str(), "Syntax Error", MB_ICONERROR);
            }

            return false;
        }
    }
    // Print out Runtime Error Messages
    catch (nu::runtime_error_t& e) {
        if (!dump_err_msg)
            return false;

        int line = e.get_line_num();
        line = line <= 0 ? g_editor.interpreter().get_cur_line_n() : line;

        char lbuf[2048] = { 0 };

        _snprintf(lbuf, sizeof(lbuf) - 1, "Error #%i at %i %s\n",
            e.get_error_code(), line, e.what());

        show_error_line(line);
        g_editor.add_info(lbuf, CFM_BOLD | CFM_ITALIC);

        ::MessageBox(get_main_hwnd(), lbuf, "Syntax Error", MB_ICONERROR);

        return false;
    }
    // Print out Syntax Error Messages
    catch (std::exception& e) {
        if (!dump_err_msg)
            return false;

        char lbuf[2048] = { 0 };

        const auto line = g_editor.interpreter().get_cur_line_n();

        if (line > 0) {
            _snprintf(
                lbuf, sizeof(lbuf) - 1, "At line %i: %s\n", line, e.what());

            show_error_line(line);
        } else {
            _snprintf(lbuf, sizeof(lbuf) - 1, "%s\n", e.what());
        }

        g_editor.add_info(lbuf, CFM_BOLD | CFM_ITALIC);

        ::MessageBox(get_main_hwnd(), lbuf, "Error", MB_ICONERROR);

        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

LRESULT nu::editor_t::get_current_line() const noexcept
{
    return send_command(
               SCI_LINEFROMPOSITION, send_command(SCI_GETCURRENTPOS, 0, 0), 0)
        + 1;
}


/* -------------------------------------------------------------------------- */

LRESULT nu::editor_t::get_current_colum() const noexcept
{
    return send_command(SCI_GETCOLUMN, send_command(SCI_GETCURRENTPOS, 0, 0), 0)
        + 1;
}


/* -------------------------------------------------------------------------- */

LRESULT nu::editor_t::get_current_position() const noexcept
{
    return send_command(SCI_GETCURRENTPOS, 0, 0);
}


/* -------------------------------------------------------------------------- */

LRESULT nu::editor_t::get_current_style() const noexcept
{
    return send_command(SCI_GETSTYLEAT, get_current_position(), 0);
}


/* -------------------------------------------------------------------------- */

int nu::editor_t::get_fold_level() const noexcept
{
    int level = (send_command(SCI_GETFOLDLEVEL, get_current_line(), 0))
        & SC_FOLDLEVELNUMBERMASK;

    return level - 1024;
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::find_next_bookmark()
{
    auto line = send_command(SCI_MARKERNEXT, get_current_line(), 0xffff);

    if (line >= 0)
        send_command(SCI_GOTOLINE, line, 0);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::find_prev_bookmark()
{
    auto line
        = send_command(SCI_MARKERPREVIOUS, get_current_line() - 2, 0xffff);

    if (line >= 0)
        send_command(SCI_GOTOLINE, line, 0);
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::search_forward(LPSTR szText)
{
    if (szText == NULL) {
        return false;
    }

    long pos = (long)get_current_position();

    TextToFind tf = { 0 };

    tf.lpstrText = szText;
    tf.chrg.cpMin = pos + 1;
    tf.chrg.cpMax = long(send_command(SCI_GETLENGTH, 0, 0));

    pos = long(send_command(SCI_FINDTEXT, _search_flags, (LPARAM)&tf));

    if (pos >= 0) {
        ::SetFocus(_hwnd_editor);

        go_to_pos(pos);

        send_command(SCI_SETSEL, tf.chrgText.cpMin, tf.chrgText.cpMax);
        send_command(SCI_FINDTEXT, _search_flags, (LPARAM)&tf);

        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::search_backward(LPSTR szText)
{
    if (szText == NULL)
        return false;

    long pos = (long)get_current_position();
    long lMinSel = (long)get_selection_begin();

    TextToFind tf = { 0 };

    tf.lpstrText = szText;

    if (lMinSel >= 0)
        tf.chrg.cpMin = lMinSel - 1;
    else
        tf.chrg.cpMin = pos - 1;

    tf.chrg.cpMax = 0;

    pos = (long)send_command(SCI_FINDTEXT, _search_flags, (LPARAM)&tf);

    if (pos >= 0) {
        ::SetFocus(_hwnd_editor);

        go_to_pos(pos);

        send_command(SCI_SETSEL, tf.chrgText.cpMin, tf.chrgText.cpMax);
        send_command(SCI_FINDTEXT, _search_flags, (LPARAM)&tf);

        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

std::string nu::editor_t::get_selection()
{
    long sel_len = long((get_selection_end() - get_selection_begin()) + 1);

    if (sel_len > 0) {
        std::vector<char> buf(sel_len + 1);

        buf[0] = '\0';

        send_command(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(buf.data()));

        std::string str;

        std::copy(buf.begin(), buf.end(), std::back_inserter(str));

        return str;
    }

    return std::string();
}


/* -------------------------------------------------------------------------- */

int nu::editor_t::replace_all(
    LPCSTR szFind, LPCSTR szReplace, BOOL bUseSelection)
{
    int nCount = 0;
    // different branches for replace in selection or total file
    if (bUseSelection) {
        // Get starting selection range
        long length = 0;
        long begin = long(get_selection_begin());
        long end = long(get_selection_end());

        // Set target to selection
        send_command(SCI_SETTARGETSTART, begin);
        send_command(SCI_SETTARGETEND, end);

        // Try to find text in target for the first time
        long pos = long(
            send_command(SCI_SEARCHINTARGET, strlen(szFind), (LPARAM)szFind));

        // loop over selection until end of selection
        // reached - moving the target start each time
        while (pos < end && pos >= 0) {
            const auto szlen = strlen(szReplace);

            length = _search_flags & SCFIND_REGEXP
                ? (long)send_command(
                      SCI_REPLACETARGETRE, szlen, (LPARAM)szReplace)
                : (long)send_command(
                      SCI_REPLACETARGET, szlen, (LPARAM)szReplace);

            // the end of the selection was changed - recalc the end
            end = long(get_selection_end());

            // move start of target behind last change and end of target to new
            // end of selection
            send_command(SCI_SETTARGETSTART, pos + length);
            send_command(SCI_SETTARGETEND, end);

            // find again - if nothing found loop exits
            pos = long(send_command(
                SCI_SEARCHINTARGET, strlen(szFind), (LPARAM)szFind));
            nCount++;
        }
    } else {
        // start with first and last char in buffer
        long length = 0;
        long begin = 0;
        long end = long(send_command(SCI_GETTEXTLENGTH, 0, 0));
        //    set target to selection
        send_command(SCI_SETTARGETSTART, begin, 0);
        send_command(SCI_SETTARGETEND, end, 0);

        // try to find text in target for the first time
        long pos = long(
            send_command(SCI_SEARCHINTARGET, strlen(szFind), (LPARAM)szFind));

        // loop over selection until end of selection reached - moving the
        // target start each time
        while (pos < end && pos >= 0) {
            const auto szlen = strlen(szReplace);

            // check for regular expression flag
            length = _search_flags & SCFIND_REGEXP
                ? (long)send_command(
                      SCI_REPLACETARGETRE, szlen, (LPARAM)szReplace)
                : (long)send_command(
                      SCI_REPLACETARGET, szlen, (LPARAM)szReplace);

            // the end of the selection was changed - recalc the end
            end = long(send_command(SCI_GETTEXTLENGTH, 0, 0));

            // move start of target behind last change and end of target to new
            // end of buffer
            send_command(SCI_SETTARGETSTART, pos + length);
            send_command(SCI_SETTARGETEND, end);

            // find again - if nothing found loop exits
            pos = long(send_command(
                SCI_SEARCHINTARGET, strlen(szFind), (LPARAM)szFind));
            nCount++;
        }
    }
    return nCount;
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::create_funcs_menu() noexcept
{
    const rt_prog_ctx_t& prog_ctx = g_editor.interpreter().get_rt_ctx();
    const auto& prototypes = prog_ctx.proc_prototypes.data;

    std::string list;

    HMENU hmenu = GetMenu(get_main_hwnd());

    hmenu = GetSubMenu(hmenu, IDM_MENU_SEARCH_POS);

    if (_func_submenu) {
        UINT item = GetMenuItemCount(hmenu);
        DeleteMenu(hmenu, item - 1, MF_BYPOSITION);
    }

    _func_submenu = CreatePopupMenu();

    _func_map.clear();
    int id = 0;

    for (const auto& f : prototypes) {
        int idm = IDM_INTERPRETER_BROWSER_FUN + id;

        _func_map.insert(std::make_pair(idm, f.second.first.get_line()));

        ++id;

        AppendMenu(_func_submenu, MF_STRING, idm, f.first.c_str());
    }

    AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)_func_submenu,
        "Go to procedure");
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::show_splash()
{
    HANDLE image = ::LoadBitmap(
        GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_NUBASIC_SPLASH));

    if (!image)
        return;

    const int wdx = 400;
    const int wdy = 170 + GetSystemMetrics(SM_CYCAPTION);

    RECT r = { 0 };
    GetClientRect(GetDesktopWindow(), &r);

    auto wx = (r.right - r.left - wdx) / 2;
    auto wy = (r.bottom - r.top - wdy) / 2;

    HWND hwnd = ::CreateWindowA("STATIC", EDITOR_INFO, WS_VISIBLE, wx, wy, wdx,
        wdy, NULL, NULL, NULL, NULL);

    HDC hdc = GetDC(hwnd);

    HDC hdcMem = ::CreateCompatibleDC(hdc);
    auto hbmOld = ::SelectObject(hdcMem, (HGDIOBJ)image);

    BITMAP bm = { 0 };
    ::GetObject(image, sizeof(bm), &bm);

    GetClientRect(hwnd, &r);

    int dx = r.right - r.left;
    int dy = r.bottom - r.top;

    auto ret = ::BitBlt(hdc, (dx - bm.bmWidth) / 2, (dy - bm.bmHeight) / 2,
        bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    Sleep(3000);

    ::SelectObject(hdcMem, hbmOld);
    ::DeleteDC(hdcMem);

    ::ReleaseDC(hwnd, hdc);

    ::DestroyWindow(hwnd);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::alloc_console()
{
    HWND hwnd = ::GetConsoleWindow();

    if (hwnd)
        return;

    AllocConsole();
    SetConsoleTitle("nuBASIC - Press CTRL+C to break execution");

    hwnd = ::GetConsoleWindow();
    if (hwnd != NULL) {
        HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);

        if (hMenu != NULL)
            DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
    }

    COORD dwSize; // new screen buffer size
    dwSize.X = 100;
    dwSize.Y = 1000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), dwSize);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    // Clear the error state for each of the C++ standard stream objects.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::start_debugging(dbg_flg_t flg)
{
    if (_need_build && !rebuild_code(true))
        return;

    reset_all_breakpoints();
    remove_prog_cnt_marker();

    alloc_console();

    g_editor.interpreter().register_break_event();

    switch (flg) {
    case dbg_flg_t::NORMAL_EXECUTION:
        exec_interpreter_cmd("run", true);
        break;
    case dbg_flg_t::CONTINUE_EXECUTION:
        exec_interpreter_cmd("cont", true);
        break;
    case dbg_flg_t::SINGLE_STEP_EXECUTION:
        exec_interpreter_cmd("ston", false);
        exec_interpreter_cmd("cont", false);
        exec_interpreter_cmd("stoff", false);
        break;
    }

    show_execution_point(interpreter().get_cur_line_n());
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::show_execution_point(int line) noexcept
{
    remove_prog_cnt_marker();

    if (line < 1)
        line = 1;

    auto endpos = send_command(SCI_GETLINEENDPOSITION, line - 1, 0) + 1;

    while (!interpreter().has_runnable_stmt(line) && line < endpos)
        ++line;

    if (send_command(SCI_POSITIONFROMLINE, line - 1, 0) >= 0) {
        const auto l = int(marker_t::LINESELECTION);

        send_command(SCI_MARKERDEFINE, l, SC_MARK_BACKGROUND);
        send_command(SCI_MARKERSETBACK, l, RGB(0, 255, 0));
        send_command(SCI_MARKERADD, line - 1, l);


        const auto m = int(marker_t::PROGCOUNTER);

        send_command(SCI_MARKERDEFINE, m, SC_MARK_ARROW);
        send_command(SCI_MARKERSETFORE, m, RGB(0, 0, 0));
        send_command(SCI_MARKERSETBACK, m, RGB(0, 0, 255));

        send_command(SCI_MARKERADD, line - 1, m);

        go_to_line(line);

        update_ui();
    } else {
        send_command(SCI_MARKERDELETE, line - 1, int(marker_t::LINESELECTION));
        send_command(SCI_MARKERDELETE, line - 1, int(marker_t::PROGCOUNTER));
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::show_error_line(int line) noexcept
{
    remove_prog_cnt_marker();

    if (line < 1)
        line = 1;

    if (send_command(SCI_POSITIONFROMLINE, line - 1, 0) >= 0) {
        const auto l = int(marker_t::LINESELECTION);

        send_command(SCI_MARKERDEFINE, l, SC_MARK_BACKGROUND);
        send_command(SCI_MARKERSETBACK, l, RGB(255, 0, 0));
        send_command(SCI_MARKERADD, line - 1, l);

        go_to_line(line);

        update_ui();
    } else {
        send_command(SCI_MARKERDELETE, line - 1, int(marker_t::LINESELECTION));
    }

    return true;
}


/* -------------------------------------------------------------------------- */

static bool RunningDlgWndProc_terminate = false;

LRESULT CALLBACK RunningDlgWndProc(
    HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        RunningDlgWndProc_terminate = false;
        SetTimer(hDlg, 0, 1000, 0);
        return TRUE;

    case WM_TIMER:
        SetWindowPos(
            GetConsoleWindow(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        if (RunningDlgWndProc_terminate) {
            KillTimer(hDlg, 0);
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        break;
    }

    return FALSE;
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::show_running_dialog()
{
    DialogBox(_hInstance, (LPCTSTR)IDD_DIALOG_RUNNING, get_main_hwnd(),
        (DLGPROC)RunningDlgWndProc);
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::exec_interpreter_cmd(const std::string& cmd, bool bg_mode)
{
    auto async_fn = [&]() {

        try {
            g_editor.interpreter().exec_command(cmd);
            RunningDlgWndProc_terminate = true;
            return true;
        } catch (nu::runtime_error_t& e) {
            char buf[2048] = { 0 };
            int line = e.get_line_num();
            line = line <= 0 ? g_editor.interpreter().get_cur_line_n() : line;

            _snprintf(buf, sizeof(buf) - 1, "Runtime Error #%i at %i %s\n",
                e.get_error_code(), line, e.what());

            g_editor.add_info(buf, CFM_BOLD | CFM_ITALIC);
            MessageBox(get_main_hwnd(), buf, "Runtime Error", MB_ICONERROR);

        } catch (std::exception& e) {
            char buf[2048] = { 0 };

            if (g_editor.interpreter().get_cur_line_n() > 0)
                _snprintf(buf, sizeof(buf) - 1, "At line %i: %s\n",
                    g_editor.interpreter().get_cur_line_n(), e.what());
            else
                _snprintf(buf, sizeof(buf) - 1, "%s\n", e.what());

            MessageBox(get_main_hwnd(), buf, "Runtime Error", MB_ICONERROR);
            g_editor.add_info(buf, CFM_BOLD | CFM_ITALIC);
        }

        RunningDlgWndProc_terminate = true;
        return false;
    };

    if (bg_mode) {
        std::thread t(async_fn);

        add_info("Run Program\n", CFM_ITALIC);

        show_running_dialog();

        g_editor.interpreter().set_step_mode(true);

        t.join();

        g_editor.interpreter().set_step_mode(false);

        add_info("Stop Program\n", CFM_ITALIC);

        return true;
    }

    return async_fn();
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::evaluate_expression(const std::string& expression)
{
    if (expression.empty()) {
        send_command(SCI_ANNOTATIONCLEARALL);
        return true;
    }

    std::string expr = "__eval_export \"";
    expr += expression;
    expr += "\" ";

    if (exec_interpreter_cmd(expr, false)) {
        auto result = interpreter().get_rt_ctx().exported_result.to_str();

        std::string annotation = "\r\n";
        annotation += expression + " -> " + result + "\r\n";

        send_command(SCI_ANNOTATIONCLEARALL);
        send_command(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_BOXED);
        send_command(
            SCI_ANNOTATIONSETSTYLE, get_current_line() - 1, SCE_B_DOCLINE);
        send_command(SCI_ANNOTATIONSETTEXT, get_current_line() - 1,
            (LPARAM)annotation.c_str());

        SetTimer(get_main_hwnd(), TIMER_EVAL_SELECTION, 3000, 0);

        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::eval_sel()
{
    std::string sel = get_selection();
    std::string qsel;

    for (size_t i = 0; i < sel.length(); ++i) {
        if (sel[i] == '\\') {
            qsel += "\\\\";
        } else if (sel[i] == '"') {
            qsel += '\\';
            qsel += '"';
        } else if (sel[i] >= 20)
            qsel += sel[i];
    }

    evaluate_expression(qsel);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::show_ctx_help()
{
    std::string sel = get_selection();

    if (sel.empty() || sel[0] == '\0') {
        g_editor.send_command(SCI_ANNOTATIONCLEARALL);
        return;
    }

    if (sel.size() <= 32) {
        auto help_text = builtin_help_t::get_instance().help(sel);

        if (!help_text.empty()) {
            send_command(SCI_ANNOTATIONCLEARALL);
            send_command(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_BOXED);
            send_command(SCI_ANNOTATIONSETSTYLE, get_current_line() - 1,
                SCE_B_PREPROCESSOR);
            send_command(SCI_ANNOTATIONSETTEXT, get_current_line() - 1,
                (LPARAM)help_text.c_str());
        }

        KillTimer(get_main_hwnd(), TIMER_CTX_HELP);
        SetTimer(get_main_hwnd(), TIMER_CTX_HELP, 3000, 0);
    }
}


/* -------------------------------------------------------------------------- */

static std::vector<std::string> split(const std::string& s, char delim = ' ')
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
        elems.push_back(item);

    return elems;
}


/* -------------------------------------------------------------------------- */

bool nu::editor_t::rebuild_code(bool show_err_msg) noexcept
{
    remove_prog_cnt_marker();

    auto doc_size = send_command(SCI_GETLENGTH);

    if (doc_size <= 0)
        return true;

    RECT rcClient; // Client area of parent window.
    GetClientRect(_h_splitter, &rcClient);

    int cyVScroll = GetSystemMetrics(SM_CYVSCROLL);

    HWND hwndPB
        = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE,
            rcClient.left, rcClient.bottom - cyVScroll, rcClient.right,
            cyVScroll, _h_splitter, (HMENU)0, get_instance_handle(), NULL);


    int cb = int(doc_size / 10);

    // Set the range and increment of the progress bar.
    SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, cb));
    SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)1, 0);

    std::vector<char> data(doc_size + 1);
    get_text_range(0, int(doc_size), data.data());

    decltype(doc_size) i = 0;
    std::string line;
    int line_num = 1;
    g_editor.interpreter().clear_all();
    bool old_style_prog = false;

    remove_funcs_menu();

    bool first_is_special_comment = false;

    while (i < doc_size) {
        if ((i % 10) == 0)
            SendMessage(hwndPB, PBM_STEPIT, 0, 0);

        char ch = data[i];

        if (ch >= ' ')
            line += ch;

        if (i >= (doc_size - 1) || ch == '\n'
            || (ch == '\r' && i < doc_size && data[i + 1] == '\n')) {
            if (ch == '\r')
                ++i;

            if (line_num == 1 && line.size() > 2 && line.substr(0, 2) == "#!") {
                first_is_special_comment = true;
            } else if (line_num == 1
                || (line_num == 2 && first_is_special_comment)) {
                try {
                    auto tokens = split(line);
                    if (!tokens.empty()) {
                        auto lnum = std::stoi(tokens[0]);
                        old_style_prog = lnum >= 1;
                    }
                } catch (...) {
                }
            }

            auto res = build_basic_line(
                line, old_style_prog ? 0 : line_num, show_err_msg);

            ::SetWindowText(get_main_hwnd(), line.c_str());

            if (!res) {
                DestroyWindow(hwndPB);
                return false;
            }

            line.clear();
            ++line_num;
        }

        ++i;
    }

    // Ensure that lines are mapped 1:1 with editing lines
    if (old_style_prog)
        interpreter().exec_command("renum 1");

    set_title();
    create_funcs_menu();
    DestroyWindow(hwndPB);

    _need_build = false;

    g_editor.add_info("Build succeeded\n", CFM_ITALIC);


    return true;
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::replace_searching_text(PCSTR szText)
{
    if (szText == NULL)
        return;

    send_command(SCI_TARGETFROMSELECTION, 0, 0);
    if (_search_flags & SCFIND_REGEXP)
        send_command(SCI_REPLACETARGETRE, strlen(szText), (LPARAM)szText);
    else
        send_command(SCI_REPLACETARGET, strlen(szText), (LPARAM)szText);
}


/* -------------------------------------------------------------------------- */

nu::editor_t::editor_t()
    : _hInstance(0)
    , _current_dialog(0)
    , _hwnd_main(0)
    , _hwnd_editor(0)
    , _is_dirty(false)
    , _need_build(true)
{
    _full_path_str.clear();
    memset(_find_str, 0, sizeof(_find_str));
    memset(_replace_str, 0, sizeof(_replace_str));

    GetObject(GetStockObject(SYSTEM_FONT), sizeof(_logfont), &_logfont);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::get_text_range(int start, int end, char* text) const
{
    TEXTRANGE tr = { 0 };

    tr.chrg.cpMin = start;
    tr.chrg.cpMax = end;
    tr.lpstrText = text;

    SendMessage(
        _hwnd_editor, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::set_title()
{
    std::string s_title = "nuBASIC";
    s_title += " - File name: '";
    s_title += _full_path_str + "'";

    ::SetWindowText(_hwnd_main, s_title.c_str());

    if (g_info) {
        std::stringstream os;
        os << "Ln " << get_current_line() << "\r\n";
        os << "Col " << get_current_colum() << "\r\n";
        if (g_editor.is_dirty())
            os << "<Mod>";
        g_info->update(os);
    }
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::set_new_document(bool clear_title)
{
    send_command(SCI_CLEARALL);
    send_command(EM_EMPTYUNDOBUFFER);

    if (clear_title) {
        _full_path_str.clear();
        set_title();

        add_info("New Program\n", CFM_ITALIC);
    }

    _is_dirty = false;
    _need_build = true;
    send_command(SCI_SETSAVEPOINT);

    remove_all_bookmarks();
    remove_all_breakpoints();
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::open_document_file(const char* fileName)
{
    std::string old_file_name = _full_path_str;

    _full_path_str = fileName;

    // Remove double quote characters from begin and end of the path string
    if (_full_path_str.size() >= 2) {
        if (_full_path_str.c_str()[0] == '\"') {
            _full_path_str
                = _full_path_str.substr(1, _full_path_str.size() - 1);

            if (_full_path_str.c_str()[_full_path_str.size() - 1] == '\"')
                _full_path_str
                    = _full_path_str.substr(0, _full_path_str.size() - 1);
        }
    }

    FILE* fp = fopen(_full_path_str.c_str(), "rb");

    if (fp) {
        set_new_document(false);

        send_command(SCI_CANCEL);
        send_command(SCI_SETUNDOCOLLECTION, 0);

        set_title();

        enum { READ_BLOCKSIZE = 128 * 1024 };

        char data[READ_BLOCKSIZE] = { 0 };

        int lenFile = int(fread(data, 1, sizeof(data), fp));

        while (lenFile > 0) {
            send_command(SCI_ADDTEXT, lenFile,
                reinterpret_cast<LPARAM>(static_cast<char*>(data)));

            lenFile = int(fread(data, 1, sizeof(data), fp));
        }

        fclose(fp);

        std::string msg = "Load '";
        msg += _full_path_str;
        msg += "'\n";

        add_info(msg, CFM_ITALIC);
    } else {
        std::string msg = "Could not open file \"";

        msg += _full_path_str + "\".";

        add_info(msg, CFM_BOLD | CFM_ITALIC);

        MessageBox(_hwnd_main, msg.c_str(), editor::application_name, MB_OK);

        _full_path_str = old_file_name;
        set_title();
    }

    send_command(SCI_SETUNDOCOLLECTION, 1);

    ::SetFocus(_hwnd_editor);

    send_command(EM_EMPTYUNDOBUFFER);
    send_command(SCI_SETSAVEPOINT);
    send_command(SCI_GOTOPOS, 0);

    remove_funcs_menu();
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::open_document()
{
    std::vector<char> open_file_name(MAX_PATH);
    open_file_name[0] = '\0';

    OPENFILENAME ofn = { sizeof(OPENFILENAME) };

    ofn.hwndOwner = _hwnd_main;
    ofn.hInstance = _hInstance;
    ofn.lpstrFile = open_file_name.data();
    ofn.nMaxFile = (DWORD)open_file_name.size() - 1;

    ofn.lpstrFilter = filter;
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrTitle = "Open File";
    ofn.Flags = OFN_HIDEREADONLY;

    if (::GetOpenFileName(&ofn))
        open_document_file(open_file_name.data());
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::save_document()
{
    if (_full_path_str.empty())
        _full_path_str = EDITOR_NONAME_FILE;

    save_file(_full_path_str);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::save_document_as()
{
    char openName[MAX_PATH] = "\0";
    strncpy(openName, _full_path_str.c_str(), MAX_PATH - 1);

    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.hwndOwner = _hwnd_main;
    ofn.hInstance = _hInstance;
    ofn.lpstrFile = openName;
    ofn.nMaxFile = sizeof(openName);
    ofn.lpstrTitle = "Save File";
    ofn.lpstrFilter = filter;
    ofn.Flags = OFN_HIDEREADONLY;

    if (::GetSaveFileName(&ofn)) {
        _full_path_str = openName;

        set_title();
        save_file(_full_path_str);

        ::InvalidateRect(_hwnd_editor, 0, 0);
    }
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::save_file(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "wb");

    if (fp) {
        auto doc_size = send_command(SCI_GETLENGTH);

        std::vector<char> data(doc_size + 1);
        get_text_range(0, int(doc_size), data.data());

        fwrite(data.data(), doc_size, 1, fp);
        fclose(fp);

        send_command(SCI_SETSAVEPOINT);

        std::string msg = "Save '" + filename + "'\n";
        add_info(msg, CFM_ITALIC);
    } else {
        std::string msg = "Could not save file \"" + filename + "\".";
        add_info(msg + "\n", CFM_ITALIC | CFM_BOLD);
        MessageBox(_hwnd_main, msg.c_str(), editor::application_name, MB_OK);
    }
}


/* -------------------------------------------------------------------------- */

int nu::editor_t::save_if_unsure()
{
    if (_is_dirty) {
        bool save_as_dialog = false;

        if (_full_path_str.empty()) {
            _full_path_str = EDITOR_NONAME_FILE;
            save_as_dialog = true;
        }

        std::string msg = "Save changes to \"" + _full_path_str + "\"?";

        int decision = MessageBox(
            _hwnd_main, msg.c_str(), editor::application_name, MB_YESNOCANCEL);

        if (decision == IDYES) {
            if (save_as_dialog)
                save_document_as();
            else
                save_document();
        }

        return decision;
    }

    return IDYES;
}


/* -------------------------------------------------------------------------- */

static bool exec_process(const std::string& cmd_line)
{
    STARTUPINFO StartInfo;
    memset(&StartInfo, 0, sizeof(StartInfo));

    char cmd[MAX_PATH] = { 0 };
    strncpy(cmd, cmd_line.c_str(), MAX_PATH - 1);

    PROCESS_INFORMATION proc_info;

    BOOL bFuncRetn = CreateProcess(NULL,
        cmd, // command line
        NULL, // process security attributes
        NULL, // primary thread security attributes
        NULL, // handles are inherited
        0, // creation flags
        NULL, // use parent's environment
        NULL, // use parent's current directory
        &StartInfo, // STARTUPINFO pointer
        &proc_info); // receives PROCESS_INFORMATION

    return bFuncRetn != FALSE;
}


/* -------------------------------------------------------------------------- */

static bool stop_process(const std::string& cmd_line)
{
    HWND h = FindWindow(0, "nuBASIC");

    if (!h)
        return false;

    DWORD processId = 0;

    if (!GetWindowThreadProcessId(h, &processId))
        return false;

    HANDLE tmpHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);

    if (!tmpHandle)
        return false;

    return TerminateProcess(tmpHandle, 0) != FALSE;
}


/* -------------------------------------------------------------------------- */

BOOL CALLBACK DlgProc_GotoLine(
    HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

    switch (Msg) {
    case WM_INITDIALOG: {
        std::string text = std::to_string(g_editor.goto_line());
        SetWindowText(GetDlgItem(hWnd, IDC_LINE), text.c_str());

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            char text[32] = { 0 };
            GetWindowText(GetDlgItem(hWnd, IDC_LINE), text, sizeof(text) - 1);

            g_editor.goto_line() = std::stoi(text);

            EndDialog(hWnd, IDOK);
            break;
        }
        case IDCANCEL:
            EndDialog(hWnd, IDCANCEL);
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}


//------------------------------------------------------------------------------

void nu::editor_t::exec_command(int id)
{
    switch (id) {
    case IDM_FILE_NEW:
        if (save_if_unsure() != IDCANCEL)
            set_new_document();
        break;

    case IDM_FILE_OPEN:
        if (save_if_unsure() != IDCANCEL)
            open_document();
        break;

    case IDM_FILE_SAVE:
        save_document();
        break;

    case IDM_FILE_SAVEAS:
        save_document_as();
        break;

    case IDM_FILE_EXIT:
        if (save_if_unsure() != IDCANCEL)
            ::PostQuitMessage(0);
        break;

    case IDM_EDIT_UNDO:
        send_command(WM_UNDO);
        break;

    case IDM_EDIT_REDO:
        send_command(SCI_REDO);
        break;

    case IDM_EDIT_CUT:
        send_command(WM_CUT);
        break;

    case IDM_EDIT_COPY:
        send_command(WM_COPY);
        break;

    case IDM_EDIT_PASTE:
        send_command(WM_PASTE);
        break;

    case IDM_EDIT_DELETE:
        send_command(WM_CLEAR);
        break;

    case IDM_EDIT_SELECTALL:
        send_command(SCI_SELECTALL);
        break;

    case IDM_INTERPRETER_BUILD:
        rebuild_code(true);
        break;

    case IDM_DEBUG_START:
        start_debugging();
        break;

    case IDM_DEBUG_CONT:
        continue_debugging();
        break;

    case IDM_DEBUG_STEP:
        singlestep_debugging();
        break;

    case IDM_DEBUG_EVALSEL:
        eval_sel();
        break;

    case IDM_DEBUG_GOTO_PC:
        show_execution_point(interpreter().get_cur_line_n());
        break;

    case IDM_DEBUG_TOGGLEBRK:
        toggle_breakpoint(long(get_current_line()));
        break;

    case IDM_DEBUG_REMOVEALLBRK:
        remove_all_breakpoints();
        break;

    case IDM_DEBUG_TOPMOST:
        SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
        break;

    case IDM_DEBUG_NOTOPMOST:
        SetWindowPos(GetConsoleWindow(), HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
        break;

    case IDM_DEBUG_RUN:
        if (!_full_path_str.empty()) {
            save_if_unsure();
            std::string nubasic_exe = "nubasic -e \"" + _full_path_str + "\"";
            std::replace(nubasic_exe.begin(), nubasic_exe.end(), '\\', '/');
            if (!exec_process(nubasic_exe.c_str())) {
                add_info("Error loading nuBasic", CFM_BOLD | CFM_ITALIC);

                MessageBox(get_main_hwnd(), "Error loading nuBasic",
                    "Run Interpreter", MB_ICONASTERISK | MB_OK);
            }
        } else {
            auto decision = MessageBox(get_main_hwnd(),
                "Source file not specified, proceed anyway ?",
                "Run Interpreter", MB_YESNOCANCEL);

            if (decision == IDYES) {
                if (!exec_process("nubasic.exe")) {
                    add_info("Error loading nuBasic", CFM_BOLD | CFM_ITALIC);
                    MessageBox(get_main_hwnd(), "Error loading nuBasic",
                        "Run Interpreter", MB_ICONASTERISK | MB_OK);
                }
            }
        }
        break;

    case IDC_AUTOCOMPLETE:
        send_command(
            SCI_AUTOCSHOW, 0, reinterpret_cast<LPARAM>(
                                  nu::editor::autocomplete_list.data.c_str()));
        break;

    case IDM_SEARCH_GOTOLINE:
        if (IDOK == DialogBox(_hInstance, MAKEINTRESOURCE(IDD_GOTOLINE),
                        _hwnd_main, (DLGPROC)DlgProc_GotoLine)) {
            go_to_line(g_editor.goto_line());
        }
        break;

    case IDM_SEARCH_FIND: {
        _find_replace_data.lStructSize = sizeof(_find_replace_data);
        _find_replace_data.hInstance = NULL;
        _find_replace_data.hwndOwner = _hwnd_main;

        std::string selection = g_editor.get_selection();

        if (!selection.empty()) {
            strncpy(_find_str, selection.c_str(), sizeof(_find_str) - 1);
        }

        _find_replace_data.lpstrFindWhat = _find_str;
        _find_replace_data.lpstrReplaceWith = _replace_str;

        _find_replace_data.wFindWhatLen = sizeof(_find_str) - 1;
        _find_replace_data.wReplaceWithLen = sizeof(_replace_str) - 1;
        _find_replace_data.Flags = 0;

        _invert_search_direction = false;

        HWND find_handle = FindText(&_find_replace_data);
        g_editor.set_current_dialog(find_handle);
        break;
    }

    case IDM_SEARCH_FINDANDREPLACE: {
        std::string selection = g_editor.get_selection();

        if (!selection.empty())
            strncpy(_find_str, selection.c_str(), sizeof(_find_str) - 1);

        _find_replace_data.lStructSize = sizeof(_find_replace_data);
        _find_replace_data.hInstance = NULL;
        _find_replace_data.hwndOwner = _hwnd_main;
        _find_replace_data.lpstrFindWhat = _find_str;
        _find_replace_data.lpstrReplaceWith = _replace_str;
        _find_replace_data.wFindWhatLen = sizeof(_find_str) - 1;
        _find_replace_data.wReplaceWithLen = sizeof(_replace_str) - 1;
        _find_replace_data.Flags = 0;

        _invert_search_direction = false;

        HWND replace_handle = ReplaceText(&_find_replace_data);
        g_editor.set_current_dialog(replace_handle);
        break;
    }

    case IDM_SEARCH_ADDMARKER:
        toggle_bookmark(long(get_current_line()));
        break;

    case IDM_SEARCH_DELALLMARKERS:
        remove_all_bookmarks();
        break;

    case IDM_SEARCH_FINDNEXTMARKER:
        find_next_bookmark();
        break;

    case IDM_SEARCH_FINDPREVIOUSMARKER:
        find_prev_bookmark();
        break;

    case IDM_SETTINGS_FONT: {
        CHOOSEFONT cf;

        // Setting the CHOOSEFONT structure
        cf.lStructSize = sizeof(CHOOSEFONT);
        cf.hwndOwner = (HWND)NULL;
        cf.lpLogFont = &_logfont;
        cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
        cf.rgbColors = RGB(0, 0, 0);
        cf.lCustData = 0L;
        cf.lpfnHook = (LPCFHOOKPROC)NULL;
        cf.lpTemplateName = (LPCSTR)NULL;
        cf.hInstance = (HINSTANCE)NULL;
        cf.lpszStyle = (LPSTR)NULL;
        cf.nFontType = SCREEN_FONTTYPE;
        cf.nSizeMin = 24;

        if (ChooseFont(&cf) == TRUE)
            g_editor.init_editor(_logfont.lfFaceName, abs(_logfont.lfHeight));

        break;
    }

    case IDM_SETTINGS_RESETDEFAULTS:
        g_editor.init_editor(EDITOR_DEF_FONT, EDITOR_DEF_SIZE);
        break;

    case IDM_CLEAR_INFOBOX:
        g_editor.clear_info();
        break;

    case IDM_HIDE_INFOBOX:
        g_editor.hide_info();
        break;

    case IDM_RESIZE_INFOBOX:
        g_editor.resize_info();
        break;

    case IDM_COPY_INFOBOX_CLIPBOARD:
        g_editor.copy_info_to_clipboard();
        break;

    case IDM_CTX_HELP:
        g_editor.show_ctx_help();
        break;

    case IDM_HELP_SEARCH_KEYWORD: {
        std::string selection = g_editor.get_selection();

        std::string online_help_url = about::homepage;

        // Selection can contain string with '\0' chars ... empty() method
        // returns false, but we have to consider selection empty as well
        if (strlen(selection.c_str()) > 0) {
            online_help_url += "/system/app/pages/search?scope=search-site&q=";
            online_help_url += selection;
        }

        std::replace(online_help_url.begin(), online_help_url.end(), ' ', '+');
        std::replace(online_help_url.begin(), online_help_url.end(), '\t', '+');
        std::replace(online_help_url.begin(), online_help_url.end(), '\n', '+');
        std::replace(online_help_url.begin(), online_help_url.end(), '\r', '+');

        online_help_url = "start iexplore \"" + online_help_url + "\"";

        if (0 != system(online_help_url.c_str())) {
            const char* msg = "Error loading Interet Browser";

            add_info(msg, CFM_BOLD | CFM_ITALIC);

            MessageBox(get_main_hwnd(), msg, "Error", MB_ICONASTERISK | MB_OK);
        }

        break;
    }

    case IDM_ABOUT_ABOUT:
        g_editor.show_splash();

        MessageBox(_hwnd_main, EDITOR_ABOUT_TEXT, EDITOR_INFO,
            MB_ICONINFORMATION | MB_OK);
        break;
    };
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::enable_menu_item(int id, bool enable)
{
    static const UINT enabled_style = MF_ENABLED | MF_BYCOMMAND;
    static const UINT disabled_style = MF_DISABLED | MF_GRAYED | MF_BYCOMMAND;

    ::EnableMenuItem(::GetMenu(get_main_hwnd()), id,
        enable ? enabled_style : disabled_style);
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::check_menus()
{
    enable_menu_item(IDM_FILE_SAVE, _is_dirty);
    enable_menu_item(IDM_EDIT_UNDO, send_command(EM_CANUNDO) != FALSE);
    enable_menu_item(IDM_EDIT_REDO, send_command(SCI_CANREDO) != FALSE);
    enable_menu_item(IDM_EDIT_PASTE, send_command(EM_CANPASTE) != FALSE);
}


void nu::editor_t::update_ui()
{
    long begin = (long)send_command(SCI_GETCURRENTPOS, 0, 0);
    long end = (long)send_command(SCI_BRACEMATCH, begin - 1, 0);

    if (end < 0) {
        begin = 0;
        end = -1;
    }

    send_command(SCI_BRACEHIGHLIGHT, begin - 1, end);
}


//------------------------------------------------------------------------------

void nu::editor_t::set_def_folding(int margin, long pos)
{
    // simply toggle fold
    if (margin == 2) {
        send_command(
            SCI_TOGGLEFOLD, send_command(SCI_LINEFROMPOSITION, pos, 0), 0);
    }
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::refresh() { send_command(SCI_COLOURISE, 0, -1); }


/* -------------------------------------------------------------------------- */

void nu::editor_t::notify(SCNotification* notification)
{
    switch (notification->nmhdr.code) {
    case SCN_AUTOCSELECTION:
        break;

    case SCN_SAVEPOINTREACHED:
        _is_dirty = false;
        _need_build = true;
        check_menus();
        break;

    case SCN_SAVEPOINTLEFT:
        set_dirty_flag();
        check_menus();
        break;

    case SCN_STYLENEEDED:
        break;

    case SCN_CHARADDED:
        break;

    case SCN_MODIFYATTEMPTRO:
        break;

    case SCN_KEY:
        break;

    case SCN_DOUBLECLICK:
        break;

    // called when something changes and we want to show new
    // indicator state or brace matching
    case SCN_UPDATEUI:
        update_ui();
        set_title();
        break;

    case SCN_MODIFIED:
        break;

    case SCN_MACRORECORD:
        break;

    // User clicked margin - try folding action
    case SCN_MARGINCLICK:
        set_def_folding(notification->margin, notification->position);
        break;

    case SCN_NEEDSHOWN:
        break;

    case SCN_PAINTED:
        break;

    case SCN_USERLISTSELECTION:
        break;

    case SCN_URIDROPPED:
        break;

    case SCN_DWELLSTART:
        break;

    case SCN_DWELLEND:
        break;
    }
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::set_item_style(
    int style, COLORREF fore, COLORREF back, int size, const char* face)
{
    send_command(SCI_STYLESETFORE, style, fore);
    send_command(SCI_STYLESETBACK, style, back);

    if (size >= 1)
        send_command(SCI_STYLESETSIZE, style, size);

    if (face)
        send_command(SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
}


/* -------------------------------------------------------------------------- */

void nu::editor_t::init_editor(const std::string& fontname, int height)
{

    // clear all text styles
    send_command(SCI_CLEARDOCUMENTSTYLE, 0, 0);

    // Set the number of styling bits to 7 (default is 5)
    send_command(SCI_SETSTYLEBITS, 7, 0);

    // Set the display for indetation guides to on.
    // This displays vertical lines from the beginning of
    // a code block to the end of the block
    send_command(SCI_SETINDENTATIONGUIDES, TRUE, 0);

    send_command(SCI_SETTABWIDTH, DEF_TABWIDTH, 0);
    send_command(SCI_SETINDENT, DEF_INDENT, 0);
    send_command(SCI_SETCARETPERIOD, DEF_CARETPERIOD, 0);
    send_command(SCI_SETUSETABS, 0, 0L);
    send_command(SCI_SETLEXER, SCLEX_VB);

    static std::string keywords;
    for (const auto& keyword : nu::reserved_keywords_t::list) {
        keywords += keyword;
        keywords += " ";
    }

    send_command(
        SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(keywords.c_str()));

    // Set up the global default style.
    set_item_style(
        STYLE_DEFAULT, editor::black, editor::white, height, fontname.c_str());

    // Copies global style to all others
    send_command(SCI_STYLECLEARALL);

    set_item_style(SCE_B_NUMBER, RGB(0x80, 0, 0x80));
    set_item_style(SCE_B_HEXNUMBER, RGB(0x80, 0, 0x80));

    set_item_style(SCE_B_COMMENT, RGB(0x80, 0x80, 0));
    set_item_style(SCE_B_COMMENTBLOCK, RGB(0x80, 0x80, 0));
    set_item_style(
        SCE_B_PREPROCESSOR, /*editor::red*/ editor::black, editor::yellow);
    set_item_style(
        SCE_B_DOCLINE, /*editor::red*/ editor::white, editor::darkBlue);

    set_item_style(SCE_B_LABEL, editor::offWhite, RGB(0x40, 0x40, 0x40));
    set_item_style(SCE_B_KEYWORD, editor::darkBlue);
    set_item_style(SCE_B_KEYWORD2, editor::darkBlue);
    set_item_style(SCE_B_KEYWORD3, editor::darkBlue);
    set_item_style(SCE_B_KEYWORD4, editor::darkBlue);

    set_item_style(SCE_B_STRING, editor::darkGreen);
    set_item_style(SCE_B_STRINGEOL, editor::darkGreen);

    set_item_style(SCE_B_DEFAULT, RGB(0x80, 0x80, 0x80));

    // source folding section
    // Tell the lexer that we want folding information
    // The lexer supplies "folding levels"
    send_command(SCI_SETPROPERTY, (WPARAM)("fold"), (LPARAM)("1"));
    send_command(SCI_SETPROPERTY, (WPARAM)("fold.preprocessor"), (LPARAM)("1"));
    send_command(SCI_SETPROPERTY, (WPARAM)("fold.comment"), (LPARAM)("1"));
    send_command(SCI_SETPROPERTY, (WPARAM)("fold.at.else"), (LPARAM)("1"));
    send_command(SCI_SETPROPERTY, (WPARAM)("fold.flags"), (LPARAM)("1"));
    send_command(SCI_SETPROPERTY, (WPARAM)("styling.within.preprocessor"),
        (LPARAM)("1"));
    send_command(
        SCI_SETPROPERTY, (WPARAM)("basic.default.language"), (LPARAM)("1"));

    // Tell scintilla to draw folding lines UNDER the folded line
    send_command(SCI_SETFOLDFLAGS, 16, 0);

    // Set margin 2 = folding margin to display folding symbols
    send_command(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);

    // Allow notifications for folding actions
    send_command(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);
    send_command(SCI_SETMODEVENTMASK,
        SC_MOD_CHANGEFOLD | SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);

    // Make the folding margin sensitive to folding events
    // If you click into the margin you get a notification event
    send_command(SCI_SETMARGINSENSITIVEN, 2, TRUE);

    // define a set of markers to displa folding symbols
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
    send_command(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);

    // Set the forground color for some styles
    send_command(SCI_STYLESETFORE, 0, RGB(0, 0, 0));
    send_command(SCI_STYLESETFORE, 2, RGB(0, 64, 0));
    send_command(SCI_STYLESETFORE, 5, RGB(0, 0, 255));
    send_command(SCI_STYLESETFORE, 6, RGB(200, 20, 0));
    send_command(SCI_STYLESETFORE, 9, RGB(0, 0, 255));
    send_command(SCI_STYLESETFORE, 10, RGB(255, 0, 64));
    send_command(SCI_STYLESETFORE, 11, RGB(0, 0, 0));

    // Set the backgroundcolor of brace highlights
    send_command(SCI_STYLESETBACK, STYLE_BRACELIGHT, RGB(0, 255, 0));

    // Set end of line mode to CRLF
    send_command(SCI_CONVERTEOLS, 2, 0);
    send_command(SCI_SETEOLMODE, 2, 0);

    // Set markersymbol for marker type 0 - bookmark
    send_command(SCI_MARKERDEFINE, 0, SC_MARK_CIRCLE);

    // Display all margins
    set_numbers_margin(true);
    set_folding_margin(true);
    set_selection_margin(true);

    // Refresh style
    refresh();
}


/* -------------------------------------------------------------------------- */


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (iMessage) {
    case WM_MOUSEMOVE:
        SetCursor(LoadCursor(NULL, IDC_HAND));
        break;

    case WM_CREATE: {
        InitCommonControls();

        // Load RichEdit Control Library
        HMODULE h_RichLib = LoadLibrary("RichEd32.Dll");

        if (!h_RichLib) {
            MessageBox(hWnd, "Cannot load rich edit control library", "Error",
                MB_ICONERROR);
            return 0;
        }

        g_editor.set_editor_hwnd(::CreateWindow("Scintilla", "Source",
            WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN, 0, 0, 100,
            100, hWnd, 0, g_editor.get_instance_handle(), 0));

        g_editor.init_editor(EDITOR_DEF_FONT, EDITOR_DEF_SIZE);

        ::ShowWindow(g_editor.get_editor_hwnd(), SW_SHOW);
        ::SetFocus(g_editor.get_editor_hwnd());

        g_editor.set_find_replace_msg(::RegisterWindowMessage(FINDMSGSTRING));

        auto cmdLine = g_editor.get_command_line();

        if (!cmdLine.empty())
            g_editor.open_document_file(cmdLine.c_str());

        g_toolbar = new toolbar_t(hWnd, g_editor.get_instance_handle(),
            IDI_NUBASIC_TOOLBAR, IDI_NUBASIC_TOOLBAR,
            nu::editor::toolbar_n_of_bmps, nu::editor::toolbar_buttons,
            nu::editor::toolbar_n_of_buttons);

        g_info = new txtinfobox_t(hWnd, g_editor.get_instance_handle());


        if (!g_toolbar || !g_info) {
            MessageBox(g_editor.get_main_hwnd(),
                "Fatal error creating a resource", "Error", MB_ICONERROR);

            return -1;
        }

        g_editor.create_splitter(hWnd);
        g_editor.create_search_replace_cntrls(hWnd);

        g_editor.show_splash();

        DragAcceptFiles(hWnd, TRUE);

        return 0;
    }

    case WM_TIMER:
        switch (wParam) {
        case nu::editor_t::TIMER_EVAL_SELECTION:
            KillTimer(
                g_editor.get_main_hwnd(), nu::editor_t::TIMER_EVAL_SELECTION);
            g_editor.eval_sel();
            break;

        case nu::editor_t::TIMER_CTX_HELP:
            KillTimer(g_editor.get_main_hwnd(), nu::editor_t::TIMER_CTX_HELP);
            g_editor.show_ctx_help();
            break;
        }

        return 0;

    case WM_SIZE:
        if (wParam != 1)
            g_editor.resize_info();
        return 0;

    case WM_DROPFILES:
        g_editor.on_drop_files((HDROP)wParam);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) >= IDM_INTERPRETER_BROWSER_FUN) {
            auto line = g_editor.resolve_funclinenum_from_id(LOWORD(wParam));

            if (line <= 0)
                MessageBeep(0);

            g_editor.go_to_line((long)line);
        } else {
            g_editor.exec_command(LOWORD(wParam));
        }

        g_editor.check_menus();
        return 0;

    case WM_NOTIFY:
        if (wParam == IDI_NUBASIC_TOOLBAR) {
            BOOL ret_val = g_toolbar->on_notify(hWnd, lParam);

            switch (((LPNMHDR)lParam)->code) {
            case TBN_QUERYDELETE:
            case TBN_GETBUTTONINFO:
            case TBN_QUERYINSERT:
                return ret_val;
            }
        } else {
            g_editor.notify(reinterpret_cast<SCNotification*>(lParam));
        }
        return 0;

    case WM_MENUSELECT:
        g_editor.check_menus();
        return 0;

    case WM_PAINT: {
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;

    case WM_CLOSE:
        if (g_editor.save_if_unsure() != IDCANCEL) {
            ::DestroyWindow(g_editor.get_editor_hwnd());
            ::PostQuitMessage(0);
            FreeConsole();
        }
        return 0;

    default:
        if (iMessage == g_editor.get_find_replace_msg()) {
            int flgs = g_editor.get_search_flags();

            LPFINDREPLACE lpf = ((LPFINDREPLACE)lParam);

            if (lpf->Flags & FR_MATCHCASE)
                flgs |= SCFIND_MATCHCASE;
            else
                flgs &= ~SCFIND_MATCHCASE;

            if (lpf->Flags & FR_WHOLEWORD)
                flgs |= SCFIND_WHOLEWORD;
            else
                flgs &= ~SCFIND_WHOLEWORD;

            g_editor.set_search_flags(flgs);

            if (lpf->Flags & FR_FINDNEXT) {
                bool search_result
                    = (lpf->Flags & FR_DOWN) != g_editor.get_search_direction()
                    ? g_editor.search_forward(lpf->lpstrFindWhat)
                    : g_editor.search_backward(lpf->lpstrFindWhat);

                if (!search_result
                    && IDYES == MessageBox(0, "Text not found, do you want to "
                                              "invert search direction?",
                                    "Search...", MB_YESNO)) {
                    g_editor.invert_search_direction();

                    // Invert selection and repeat searching
                    bool search_result = (lpf->Flags & FR_DOWN)
                            != g_editor.get_search_direction()
                        ? g_editor.search_forward(lpf->lpstrFindWhat)
                        : g_editor.search_backward(lpf->lpstrFindWhat);

                    if (!search_result) {
                        std::string msg = "'";
                        msg += std::string(lpf->lpstrFindWhat);
                        msg += "' not found\n";

                        g_editor.add_info(msg, CFM_BOLD | CFM_ITALIC);

                        MessageBox(0, msg.c_str(), "Search...",
                            MB_OK | MB_ICONEXCLAMATION);
                    }
                }
            }

            if (lpf->Flags & FR_REPLACE) {
                g_editor.replace_searching_text(lpf->lpstrReplaceWith);
            }

            if (lpf->Flags & FR_REPLACEALL) {
                g_editor.replace_all(
                    lpf->lpstrFindWhat, lpf->lpstrReplaceWith, FALSE);
            }

            if (lpf->Flags & FR_DIALOGTERM) {
                g_editor.set_current_dialog(0);
            }

            return 0;
        }

        return DefWindowProc(hWnd, iMessage, wParam, lParam);
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

static void RegisterWindowClass()
{
    const char resourceName[] = EDITOR_RESOURCE_NAME;

    WNDCLASSEX wndclass = { 0 };
    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = g_editor.get_instance_handle();
    wndclass.hIcon
        = LoadIcon(wndclass.hInstance, MAKEINTRESOURCE(IDI_NUBASIC_ICON));

    wndclass.hIconSm = NULL;
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = resourceName;
    wndclass.lpszClassName = nu::editor::class_name;

    if (!::RegisterClassEx(&wndclass))
        ::exit(FALSE);
}


/* -------------------------------------------------------------------------- */

//#pragma warning(disable : 4786)


/* -------------------------------------------------------------------------- */

static BOOL GetClientWindowRect(HWND hWnd, RECT& rect)
{
    HWND hParent = GetParent(hWnd);
    BOOL ret_val = GetWindowRect(hWnd, &rect);

    if (ret_val) {
        POINT left_top, right_bottom;
        left_top.x = rect.left;
        left_top.y = rect.top;
        right_bottom.x = rect.right;
        right_bottom.y = rect.bottom;

        ScreenToClient(hParent, &left_top);
        ScreenToClient(hParent, &right_bottom);

        rect.left = left_top.x;
        rect.top = left_top.y;
        rect.right = right_bottom.x;
        rect.bottom = right_bottom.y;
    }

    return ret_val;
}


namespace nu {

/* -------------------------------------------------------------------------- */

LRESULT CALLBACK HSplitterWndProc(
    HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
    PAINTSTRUCT ps;

    static RECT toolbar_rect;
    static RECT old_editor_rect;
    static RECT old_hsplitter_rect;
    static RECT old_searchbox_rect;

    static POINT old_pt;
    static POINT pt;

    static DWORD dy = 0;

    static bool bMoved = false;

    switch (Message) {
    case WM_LBUTTONDOWN:

        GetClientWindowRect(g_editor.get_editor_hwnd(), old_editor_rect);
        GetClientWindowRect(g_editor._h_splitter, old_hsplitter_rect);
        GetClientWindowRect(g_editor._h_infobox, old_searchbox_rect);
        if (g_toolbar)
            GetClientWindowRect(g_toolbar->get_hwnd(), toolbar_rect);

        SetCapture(hWnd);
        ClipCursor(NULL);
        bMoved = false;

        GetCursorPos(&old_pt);

        return 0;

    case WM_MOUSEMOVE:
        SetCursor(LoadCursor(NULL, IDC_SIZENS));

        if (wParam & MK_LBUTTON) {
            bMoved = true;

            GetCursorPos(&pt);

            dy = pt.y - old_pt.y;
            auto new_y = old_hsplitter_rect.top + dy;

            if (new_y > ULONG(toolbar_rect.bottom)) {
                MoveWindow(hWnd, old_hsplitter_rect.left,
                    old_hsplitter_rect.top + dy,
                    old_hsplitter_rect.right - old_hsplitter_rect.left,
                    old_hsplitter_rect.bottom - old_hsplitter_rect.top, TRUE);
            }
        }
        break;

    case WM_LBUTTONUP:
        MoveWindow(hWnd, old_hsplitter_rect.left, old_hsplitter_rect.top + dy,
            old_hsplitter_rect.right - old_hsplitter_rect.left,
            old_hsplitter_rect.bottom - old_hsplitter_rect.top, TRUE);

        MoveWindow(g_editor.get_editor_hwnd(), old_editor_rect.left,
            old_editor_rect.top, old_editor_rect.right - old_editor_rect.left,
            old_editor_rect.bottom - old_editor_rect.top + dy, TRUE);

        MoveWindow(g_editor._h_infobox, old_searchbox_rect.left,
            old_searchbox_rect.top + dy,
            old_searchbox_rect.right - old_searchbox_rect.left,
            old_searchbox_rect.bottom - old_searchbox_rect.top - dy, TRUE);

        InvalidateRect(g_editor.get_editor_hwnd(), NULL, TRUE);
        InvalidateRect(g_editor._h_infobox, NULL, TRUE);

        UpdateWindow(g_editor.get_editor_hwnd());
        UpdateWindow(g_editor._h_infobox);

        ClipCursor(NULL);
        ReleaseCapture();
        return 0;

    case WM_PAINT: {
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rect;
        FillRect(
            hdc, &ps.rcPaint, GetSysColorBrush(COLOR_3DFACE)); // LIGHT GRAY
        GetClientRect(hWnd, &rect);
        FrameRect(hdc, &rect, GetSysColorBrush(COLOR_3DSHADOW));
        rect.bottom = rect.top;
        FrameRect(hdc, &rect, GetSysColorBrush(COLOR_3DLIGHT)); // WHITE
        EndPaint(hWnd, &ps);
    } break;


    default:
        return DefWindowProc(hWnd, Message, wParam, lParam);
    }
    return 0;
}


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

int PASCAL WinMain(
    HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int nCmdShow)
{
    g_editor.set_instance_handle(hInstance);
    g_editor.set_command_line(lpszCmdLine);

    g_hAccTable = LoadAccelerators(hInstance, EDITOR_RESOURCE_NAME);

    if (::LoadLibrary(EDITOR_DLL_NAME) == NULL) {
        MessageBox(0, "Error loading " EDITOR_DLL_NAME, EDITOR_RESOURCE_NAME,
            MB_ICONERROR | MB_OK);
        return 0;
    }

    RegisterWindowClass();

    g_editor.set_hwnd(::CreateWindowEx(WS_EX_CLIENTEDGE, nu::editor::class_name,
        "nuBASIC", WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX
            | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
        g_editor.get_instance_handle(), 0));

    g_editor.set_title();
    ::ShowWindow(g_editor.get_main_hwnd(), nCmdShow);

    return g_winMsgProc.processWinMsg(g_hAccTable);
}
