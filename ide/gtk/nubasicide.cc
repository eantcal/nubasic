//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */


#include "nu_msgbox.h"
#include "nu_menu.h"
#include "nu_menubar.h"
#include "nu_statusbar.h"
#include "nu_accelgroup.h"
#include "nu_window.h"
#include "nu_vbox.h"
#include "nu_toolbar.h"
#include "nu_widget.h"
#include "nu_editor.h"
#include "nu_dialog_openfile.h"
#include "nu_dialog_savefile.h"
#include "nu_dialog_font.h"
#include "nu_dialog_about.h"
#include "nu_dialog_inputbox.h"
#include "nu_terminal_frame.h"
#include "nu_about.h"
#include "nu_builtin_help.h"
#include "nu_builtin_help.h"
#include "nu_exception.h"
#include "nu_interpreter.h"
#include "nu_reserved_keywords.h"

#include "mip_json_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <math.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <thread>


/* -------------------------------------------------------------------------- */

#define NU_XSTR(s) NU_STR(s)
#define NU_STR(s) #s

#define IDE_CFG_FILE "ide.cfg"
#define IDE_CFG_DIR ".nubasic"

#define IDE_DEF_FONT "Monospace"
#define IDE_DEF_FONT_SIZE 9

#define IDE_SETTINGS_DEF_FONTNAME "fontname"
#define IDE_SETTINGS_DEF_FONTSIZE "fontsize"


/* -------------------------------------------------------------------------- */

struct cfg_t {

    std::string homedir() {
        struct passwd *pw = getpwuid(getuid());
        return pw->pw_dir ? pw->pw_dir : "";
    }

    bool _mkdir(const char *path, mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)  {
        struct stat st;

        if (stat(path, &st) != 0) {
            if (mkdir(path, mode) != 0 && errno != EEXIST) {
                return false;
            }
        } else if (!S_ISDIR(st.st_mode)) {
            return false;
        }

        return true;
    }

    cfg_t() {
        //std::string cfg_dir = homedir();
        //cfg_dir+="/";
        //cfg_dir+=IDE_CFG_DIR;
        //_mkdir(cfg_dir.c_str());
    }


    bool save(const char* cfgdir) {
        return _mkdir(cfgdir);
    }

    bool load(
       const char* filename,
       mip::json_obj_t::handle_t & handle)
    {
        std::stringstream ss;
        mip::json_parser_t parser(& ss);

        std::ifstream is(filename, std::ifstream::in | std::ifstream::binary);

        if (!is.is_open() || is.bad()) {
            std::_cerr << "Cannot open " << filename << std::endl;
            return false;
        }

        std::_cout << "Loading configuration from '" << filename << "'" << std::endl;

        std::stringstream errlog;
        auto res = parser.parse(is, errlog);

        if (!res.first) {
            std::_cerr << std::endl << "JSON text is Invalid: "
                    << errlog.str() << std::endl;
            return false;
        }

        handle = std::move(res.second);

        return true;
    }
};


/* -------------------------------------------------------------------------- */

struct app_t {
    enum class marker_t {
        BOOKMARK = 0,
        BREAKPOINT = 1,
        PROGCOUNTER = 2,
        LINESELECTION = 4,
    };

    enum class dbg_flg_t {
        NORMAL_EXECUTION,
        CONTINUE_EXECUTION = 1,
        SINGLE_STEP_EXECUTION = 2
    };



    static const int black = RGB(0, 0, 0);
    static const int white = RGB(0xff, 0xff, 0xff);
    static const int red = RGB(0xff, 0, 0);
    static const int yellow = RGB(0xff, 0xff, 0xe0);
    static const int offWhite = RGB(0xff, 0xfb, 0xF0);
    static const int darkGreen = RGB(0, 0x80, 0);
    static const int darkBlue = RGB(0, 0, 0x80);

    static int dialog_goto (nu::window_t & parent) {
        int ret = -1;
        nu::dialog_inputbox_t dialog(parent, "Go to line...","Line", "1");
        if (dialog.run() == GTK_RESPONSE_OK) {
            try {
               ret = std::stoi(dialog.edit_text());
            }
            catch (...) {}
        }

        return ret;
    }


    /* -------------------------------------------------------------------------- */

    static std::vector<std::string> split(const std::string& s, char delim = ' ') {
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, delim))
            elems.push_back(item);

        return elems;
    }


    /* -------------------------------------------------------------------------- */

    void set_default_icon() {
        GdkPixbuf *pixbuf =
            gdk_pixbuf_new_from_resource("/org/gnome/name/nuBasic.ico", nullptr);

        if (pixbuf) {
            gtk_window_set_icon(GTK_WINDOW(mainwin().get_internal_obj()),pixbuf);
        }
    }


    /* -------------------------------------------------------------------------- */

    void configure_editor() {
        auto & ed = editor();

        // clear all text styles
        ed.cmd(SCI_CLEARDOCUMENTSTYLE, 0, 0);

        // Set the number of styling bits to 7 (default is 5)
        ed.cmd(SCI_SETSTYLEBITS, 7, 0);

        // Set the display for indentation guides to on.
        // This displays vertical lines from the beginning of
        // a code block to the end of the block
        ed.cmd(SCI_SETINDENTATIONGUIDES, 1, 0);

        ed.cmd(SCI_SETTABWIDTH, nu::editor_t::DEF_TABWIDTH, 0);
        ed.cmd(SCI_SETINDENT, nu::editor_t::DEF_INDENT, 0);
        ed.cmd(SCI_SETCARETPERIOD, nu::editor_t::DEF_CARETPERIOD, 0);

        ed.cmd(SCI_SETUSETABS, 0, 0L);
        ed.cmd(SCI_SETLEXER, SCLEX_VB);

        //static std::string keywords;
        //for (const auto& keyword : nu::reserved_keywords_t::list) {
        //    keywords += keyword;
        //    keywords += " ";
        //}

        //editor.cmd(SCI_SETKEYWORDS, 0, keywords.c_str());

        // Get font name and size
        auto font = cfg(IDE_SETTINGS_DEF_FONTNAME);
        auto fontsize = cfg(IDE_SETTINGS_DEF_FONTSIZE);

        if (font.empty())
            font = IDE_DEF_FONT;

        int i_fontsize = IDE_DEF_FONT_SIZE;

        try {
            i_fontsize = fontsize.empty() ? IDE_DEF_FONT_SIZE : std::stoi(fontsize);
        }
        catch (...) {}

        // Set up the global default style.
        ed.set_item_style(STYLE_DEFAULT, black, white, i_fontsize, font.c_str());

        // Copies global style to all others
        ed.cmd(SCI_STYLECLEARALL);

        ed.set_item_style(SCE_B_NUMBER, RGB(0x80, 0, 0x80));
        ed.set_item_style(SCE_B_HEXNUMBER, RGB(0x80, 0, 0x80));

        ed.set_item_style(SCE_B_COMMENT, RGB(0x80, 0x80, 0));
        ed.set_item_style(SCE_B_COMMENTBLOCK, RGB(0x80, 0x80, 0));
        ed.set_item_style(SCE_B_PREPROCESSOR, black, yellow);
        ed.set_item_style(SCE_B_DOCLINE, white, darkBlue);

        ed.set_item_style(SCE_B_LABEL, offWhite, RGB(0x40, 0x40, 0x40));
        ed.set_item_style(SCE_B_KEYWORD, darkBlue);
        ed.set_item_style(SCE_B_KEYWORD2, darkBlue);
        ed.set_item_style(SCE_B_KEYWORD3, darkBlue);
        ed.set_item_style(SCE_B_KEYWORD4, darkBlue);

        ed.set_item_style(SCE_B_STRING, darkGreen);
        ed.set_item_style(SCE_B_STRINGEOL, darkGreen);

        ed.set_item_style(SCE_B_DEFAULT, RGB(0x80, 0x80, 0x80));

        // source folding section
        // Tell the lexer that we want folding information
        // The lexer supplies "folding levels"
        ed.cmd(SCI_SETPROPERTY, "fold", "1");
        ed.cmd(SCI_SETPROPERTY, "fold.preprocessor", "1");
        ed.cmd(SCI_SETPROPERTY, "fold.comment", "1");
        ed.cmd(SCI_SETPROPERTY, "fold.at.else", "1");
        ed.cmd(SCI_SETPROPERTY, "fold.flags", "1");
        ed.cmd(SCI_SETPROPERTY, "styling.within.preprocessor", "1");
        ed.cmd(SCI_SETPROPERTY, "basic.default.language", "1");

        // Tell scintilla to draw folding lines UNDER the folded line
        ed.cmd(SCI_SETFOLDFLAGS, 16, 0);

        // Set margin 2 = folding margin to display folding symbols
        ed.cmd(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);

        // Allow notifications for folding actions
        ed.cmd(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);
        ed.cmd(SCI_SETMODEVENTMASK, SC_MOD_CHANGEFOLD | SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);

        // Make the folding margin sensitive to folding events
        // If you click into the margin you get a notification event
        ed.cmd(SCI_SETMARGINSENSITIVEN, 2, TRUE);

        // define a set of markers to display folding symbols
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
        ed.cmd(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);

        // Set the foreground colour for some styles
        ed.cmd(SCI_STYLESETFORE, 0, RGB(0, 0, 0));
        ed.cmd(SCI_STYLESETFORE, 2, RGB(0, 64, 0));
        ed.cmd(SCI_STYLESETFORE, 5, RGB(0, 0, 255));
        ed.cmd(SCI_STYLESETFORE, 6, RGB(200, 20, 0));
        ed.cmd(SCI_STYLESETFORE, 9, RGB(0, 0, 255));
        ed.cmd(SCI_STYLESETFORE, 10, RGB(255, 0, 64));
        ed.cmd(SCI_STYLESETFORE, 11, RGB(0, 0, 0));

        // Set the backgroundcolor of brace highlights
        ed.cmd(SCI_STYLESETBACK, STYLE_BRACELIGHT, RGB(0, 255, 0));

        // Set end of line mode to CRLF
        ed.cmd(SCI_CONVERTEOLS, 2, 0);
        ed.cmd(SCI_SETEOLMODE, 2, 0);

        // Set markersymbol for marker type 0 - bookmark
        ed.cmd(SCI_MARKERDEFINE, 0, SC_MARK_CIRCLE);

        // Display all margins

        ed.set_numbers_margin(true);
        ed.set_folding_margin(true);
        ed.set_selection_margin(true);

        ed.refresh();
    }


    /* ---------------------------------------------------------------------- */

    void make_editor(const nu::vbox_t& vbox) {
        static nu::editor_t editor;
        editor.set_id(0);
        vbox.container_add(editor);

        _editor_ptr = &editor;
    }


    /* ---------------------------------------------------------------------- */

    void open_document_file(const char* fileName) {
        std::string old_file_name = _full_path_str;

        _full_path_str = fileName;

        FILE* fp = fopen(_full_path_str.c_str(), "rb");

        if (fp) {
            set_new_document(false);

            editor().cmd(SCI_CANCEL);
            editor().cmd(SCI_SETUNDOCOLLECTION, 0);

            set_title();

            enum { READ_BLOCKSIZE = 128 * 1024 };

            char data[READ_BLOCKSIZE] = { 0 };

            int lenFile = int(fread(data, 1, sizeof(data), fp));

            while (lenFile > 0) {
                editor().cmd(SCI_ADDTEXT, lenFile, data);
                lenFile = int(fread(data, 1, sizeof(data), fp));
            }

            fclose(fp);

            std::string msg = "Load '";
            msg += _full_path_str;
            msg += "'\n";
        } else {
            std::string msg = "Could not open file \"";

            msg += _full_path_str + "\".";

            nu::msgbox(mainwin(), msg.c_str(), "TODO");

            _full_path_str = old_file_name;
             set_title();
        }

        editor().cmd(SCI_SETUNDOCOLLECTION, 1);

        editor().cmd(SCI_EMPTYUNDOBUFFER);
        editor().cmd(SCI_SETSAVEPOINT);
        editor().cmd(SCI_GOTOPOS, 0);
    }


    /* -------------------------------------------------------------------------- */

    void set_title() {
        std::string s_title = "nuBASIC";
        s_title += " - File name: '";
        s_title += _full_path_str + "'";

        mainwin().set_title(s_title.c_str());

        std::stringstream os;
        os << "Ln " << editor().get_current_line();
        os << " Col " << editor().get_current_colum() << "\r\n";

        if (is_dirty())
            os << "<Mod>";

        const auto txt = os.str();
        statusbar().set_text(txt.c_str());
    }

    /* -------------------------------------------------------------------------- */

    void save_document_as()
    {
        nu::dialog_savefile_t dlg(mainwin(), "Save File");

        auto response = dlg.run("", get_instance().get_working_file());

        if (response == GTK_RESPONSE_ACCEPT) {
            const char *filename = dlg.filename();

            _full_path_str = filename;

            set_title();
            save_file(_full_path_str.c_str());
        }

    }


    /* -------------------------------------------------------------------------- */

    void save_file(const char* filename)
    {
        FILE* fp = fopen(filename, "w");

        if (fp) {
            auto doc_size = editor().cmd(SCI_GETLENGTH);

            std::vector<char> data(doc_size + 1);
            editor().get_text_range(0, int(doc_size), data.data());

            fwrite(data.data(), doc_size, 1, fp);
            fclose(fp);

            editor().cmd(SCI_SETSAVEPOINT);
        } else {
            std::string msg = "Could not save file \"";
            msg += filename;
            msg += "\".";

            nu::msgbox(mainwin(), msg.c_str(), "Information", GTK_BUTTONS_OK);
        }
    }


    /* -------------------------------------------------------------------------- */

    void save_document()
    {
        if (_full_path_str.empty())
            _full_path_str = "noname.bas";

        save_file(_full_path_str.c_str());
    }


    /* -------------------------------------------------------------------------- */

    int save_if_unsure()
    {
        if (_is_dirty) {
            bool save_as_dialog = false;

            if (_full_path_str.empty()) {
                _full_path_str = "noname.bas";
                save_as_dialog = true;
            }

            std::string msg = "Save changes to \"" + _full_path_str + "\"?";

            int decision = nu::msgbox(
                mainwin(), msg.c_str(), "Confirmation", GTK_BUTTONS_YES_NO);

            if (decision == GTK_RESPONSE_YES) {
                if (save_as_dialog)
                    save_document_as();
                else
                    save_document();
            }

            return decision;
        }

        return 1;
    }


    /* -------------------------------------------------------------------------- */

    void set_new_document(bool clear_title)  {
        editor().cmd(SCI_CLEARALL);
        editor().cmd(SCI_EMPTYUNDOBUFFER);

        if (clear_title) {
            _full_path_str.clear();
            set_title();
        }

        _is_dirty = false;
        _need_build = true;
        editor().cmd(SCI_SETSAVEPOINT);

        editor().remove_all_bookmarks();
        editor().remove_all_breakpoints();
    }

    /* -------------------------------------------------------------------------- */

    static void toolbar_openfile(GtkToolItem *toolitem, GtkWindow *parentWindow) {
        menu_file_open(nullptr, parentWindow);
    }


    /* -------------------------------------------------------------------------- */

    static void dummy(GtkToolItem *toolitem, GtkWindow *parentWindow) {
        //nu::msgbox(parentWindow, "save", "save");
        nu::dialog_about_t about("program name","version","author","license","description");
    }

    static void make_toolbar(const nu::window_t& window, const nu::vbox_t& vbox) {
        nu::toolbar_t toolbar;

        toolbar.set_show_arrow();
        toolbar.set_style(GTK_TOOLBAR_ICONS);
        vbox.pack_start(toolbar);

        toolbar.add_stock_item(GTK_STOCK_NEW, "New", window, [] { get_instance().set_new_document(true); }, 0);
        toolbar.add_stock_item(GTK_STOCK_OPEN, "Open", window, toolbar_openfile, 1);
        toolbar.add_stock_item(GTK_STOCK_SAVE, "Save", window, [] { get_instance().save_document(); }, 2);
        toolbar.add_stock_item(GTK_STOCK_MEDIA_PLAY, "Run", window, [] { get_instance().start_debugging(); }, 3);
        toolbar.add_stock_item(GTK_STOCK_GOTO_LAST, "Step", window, [] { get_instance().singlestep_debugging(); }, 4);
        toolbar.add_stock_item(GTK_STOCK_GO_FORWARD, "Continue", window, [] { get_instance().continue_debugging(); }, 5);
        toolbar.add_stock_item(GTK_STOCK_MEDIA_RECORD, "Breakpoint", window, dummy, 6);
        toolbar.add_stock_item(GTK_STOCK_EXECUTE, "Build", window, [] { get_instance().rebuild_code(true); } , 7);
        toolbar.add_stock_item(GTK_STOCK_JUSTIFY_FILL, "Evaluate", window, dummy, 8);
        toolbar.add_stock_item(GTK_STOCK_FIND, "Find", window, dummy, 9);
    }


    /* -------------------------------------------------------------------------- */

    // on stop the GTK+ main loop function
    static void destroy_window(GtkWidget *window, gpointer data) {
        get_instance().quit();
        //gtk_main_quit();
    }


    /* -------------------------------------------------------------------------- */
    /*
     Return FALSE to destroy the widget.
     By returning TRUE, you can cancel a delete-event.
     This can be used to confirm quitting the application.
     */
    static gboolean delete_event_window(GtkWidget *window, GdkEvent *event, gpointer data) {
        (void) window;
        (void) event;
        (void) data;

        return (FALSE);
    }


    /* ---------------------------------------------------------------------- */

    static void menu_dummy(GtkMenuItem *fileopen, GtkWindow *parentWindow) {
        nu::msgbox(parentWindow, "Menu clicked.", "Menu");
    }


    /* ---------------------------------------------------------------------- */

    static void menu_file_open(GtkMenuItem *fileopen, GtkWindow *window) {
        (void) fileopen;

        nu::window_t nuwin((GtkWidget *)window);

        nu::dialog_openfile_t dlg(nuwin, "Open File");

        auto response = dlg.run();

        if (response == GTK_RESPONSE_ACCEPT) {
            const char *filename = dlg.filename();

            if (filename) {

                // TODO
                get_instance().set_working_file(filename);
                get_instance().open_document_file(filename);

                // TODO path
            }
        }
    }

    /* -------------------------------------------------------------------------- */

    static void menu_file_saveas(GtkMenuItem *, GtkWindow *window) {
        get_instance().save_document_as();
    }


    /* ---------------------------------------------------------------------- */

    static void menu_settings_font(GtkMenuItem *, GtkWindow *window) {
        nu::window_t nuwin((GtkWidget *)window);

        auto & app = get_instance();

        auto fontname = app.cfg(IDE_SETTINGS_DEF_FONTNAME);
        auto fontsize = app.cfg(IDE_SETTINGS_DEF_FONTSIZE);

        if (fontname.empty())
            fontname = IDE_DEF_FONT;

        int i_fontsize = IDE_DEF_FONT_SIZE;

        try {
            i_fontsize = std::stoi(fontsize);
        }
        catch (...) {}

        nu::dialog_font_t dlg(nuwin, fontname.c_str(), i_fontsize);

        auto response = dlg.run();

        if (response == GTK_RESPONSE_OK) {

            auto c_font = dlg.font();
            i_fontsize = dlg.font_size();
            std::string font = c_font ? c_font : IDE_DEF_FONT;

            app.set_cfg(IDE_SETTINGS_DEF_FONTNAME, font.c_str());
            app.set_cfg(IDE_SETTINGS_DEF_FONTSIZE, std::to_string(i_fontsize));

            app.configure_editor();
        }

    }


    /* ---------------------------------------------------------------------- */

    static int exit_main_window(GtkWidget* w, GdkEventAny* e, gpointer p) {
        (void) w;
        (void) e;
        (void) p;

        get_instance().quit();

        return 1;
    }


    /* ---------------------------------------------------------------------- */

    static void on_scintilla_notification(
        ScintillaObject * /*scintilla_obj*/,
        gint              /*dummy*/,
        struct SCNotification *notification,
        gpointer          /*user_data*/)
    {
        auto & app = get_instance();

        switch (notification->nmhdr.code) {
            case SCN_AUTOCSELECTION:
                break;

            case SCN_SAVEPOINTREACHED:
                app.set_dirty_flg(false);
                app.set_needbuild_flg();
                break;

            case SCN_SAVEPOINTLEFT:
                app.set_dirty_flg();
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
                app.editor().update_ui();
                get_instance().set_title();
                break;

            case SCN_MODIFIED:
                break;

            case SCN_MACRORECORD:
                break;

            // User clicked margin - try folding action
            case SCN_MARGINCLICK:
                app.editor().set_def_folding(
                        notification->margin,
                        notification->position);
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


    /* ---------------------------------------------------------------------- */

    void load_configuration() noexcept {
        // Read configuration
        cfg_t cfg;

        std::string cfgfile = cfg.homedir();
        cfgfile += "/";
        cfgfile += IDE_CFG_DIR;
        cfgfile += "/";
        cfgfile += IDE_CFG_FILE;

        mip::json_obj_t::handle_t jsonobj_handle;

        if (cfg.load(cfgfile.c_str(),jsonobj_handle)) {
           auto configuration = jsonobj_handle->get_object();
           if (configuration) {
               for (const auto & kv : *configuration) {
                   std::string value;

                   if (kv.second->to_string(value)) {
                       _cfg[kv.first] = value;
                   }
               }
           }
        }
    }


    /* ---------------------------------------------------------------------- */

    void quit() {
        save_if_unsure();
        gtk_main_quit();
    }


    /* ---------------------------------------------------------------------- */

    static void make_file_menu(
        nu::window_t & window,
        nu::menubar_t & menubar,
        nu::accelgroup_t& accelgroup)
    {
        // File ---------------------------------------------------------------
        nu::menu_t menu("File", menubar, accelgroup);

        menu.add_stock_item(window, GTK_STOCK_NEW, []{ get_instance().set_new_document(true); });
        menu.add_stock_item(window, GTK_STOCK_OPEN, menu_file_open);
        menu.add_separator();
        menu.add_stock_item(window, GTK_STOCK_SAVE, []{ get_instance().save_document(); });
        menu.add_stock_item(window, GTK_STOCK_SAVE_AS, menu_file_saveas);

        menu.add_separator();
        menu.add_stock_item(window, GTK_STOCK_QUIT, [](){ get_instance().quit(); });
    }


    /* ---------------------------------------------------------------------- */

    static void make_edit_menu(
        nu::window_t & window,
        nu::menubar_t & menubar,
        nu::accelgroup_t& accelgroup)
    {
        nu::menu_t menu("Edit", menubar, accelgroup);
        menu.add_stock_item(window, GTK_STOCK_UNDO,
            [](){get_instance().editor().cmd(SCI_UNDO);});

        menu.add_stock_item(window, GTK_STOCK_REDO,
            [](){get_instance().editor().cmd(SCI_REDO);});

        menu.add_separator();
        menu.add_stock_item(window, GTK_STOCK_CUT,
            [](){get_instance().editor().cmd(SCI_CUT);});

        menu.add_stock_item(window, GTK_STOCK_COPY,
            [](){get_instance().editor().cmd(SCI_COPY);});

        menu.add_stock_item(window, GTK_STOCK_PASTE,
            [](){get_instance().editor().cmd(SCI_PASTE);});

        menu.add_stock_item(window, GTK_STOCK_DELETE,
            [](){get_instance().editor().cmd(SCI_DELETEBACK);});

        menu.add_separator();

        menu.add_stock_item(window, GTK_STOCK_SELECT_ALL,
            [](){get_instance().editor().cmd(SCI_SELECTALL);});
    }


    /* ---------------------------------------------------------------------- */

    static void make_settings_menu(
        nu::window_t & window,
        nu::menubar_t & menubar,
        nu::accelgroup_t& accelgroup)
    {
        // Settings -----------------------------------------------------------
        nu::menu_t menu("Settings", menubar, accelgroup);
        menu.add_stock_item(window, GTK_STOCK_SELECT_FONT, menu_settings_font);

        menu.add_separator();

        menu.add_item(
           window,
           "Reload",
            [](){
                get_instance().load_configuration();
                get_instance().configure_editor();
            },false);

        menu.add_item(
            window,
            "Save", //TODO
            [](){
                get_instance().load_configuration();
                get_instance().configure_editor();
            },false);

        menu.add_separator();

        menu.add_item(
            window,
            "Reset default",
            [](){
                get_instance().load_configuration(); // TODO
                get_instance().configure_editor();
            }, false);
    }


    /* ---------------------------------------------------------------------- */

    static void make_search_menu(
        nu::window_t & window,
        nu::menubar_t & menubar,
        nu::accelgroup_t& accelgroup)
    {
        nu::menu_t menu("Search", menubar, accelgroup);

        menu.add_item(
            window, "Go to line...",
            [](){
                auto line = dialog_goto(get_instance().mainwin());
                if (line>0) {
                    get_instance().editor().go_to_line(line);
                }
            }, false);

        menu.add_separator();

        menu.add_stock_item(
            window, GTK_STOCK_FIND,
            [](){
                // TODO
            });

        menu.add_stock_item(
            window, GTK_STOCK_FIND_AND_REPLACE,
            [](){
                // TODO
            });

        menu.add_separator();

        menu.add_item(
            window, "Add/Remove marker",
            [](){
                auto & ed = get_instance().editor();
                ed.toggle_bookmark(long(ed.get_current_line()));
            }, false);

        menu.add_item(
            window, "Find next marker",
            [](){ get_instance().editor().find_next_bookmark(); }, false);

        menu.add_item(
            window, "Find previous marker",
            [](){ get_instance().editor().find_prev_bookmark(); }, false);

        menu.add_separator();

        menu.add_item(
            window, "Remove all markers",
            [](){ get_instance().editor().remove_all_bookmarks(); }, false);
    }


    /* ---------------------------------------------------------------------- */

    app_t(int argc, char* argv[]) {
        _this = this;

        gtk_init(&argc, &argv);

        static nu::window_t mainwindow(GTK_WINDOW_TOPLEVEL);
        _mainwin_ptr = &mainwindow;

        load_configuration();

        mainwindow.set_title("nuBASIC");
        mainwindow.maximize();
        mainwindow.on_destroy(destroy_window);
        mainwindow.on_destroy(delete_event_window);

        nu::vbox_t vbox;
        mainwindow.add_container(vbox);

        nu::menubar_t menubar;
        vbox.pack_start(menubar);

        nu::accelgroup_t accelgroup;
        mainwindow.add_accel_group(accelgroup);

        make_file_menu(mainwindow, menubar, accelgroup);
        make_edit_menu(mainwindow, menubar, accelgroup);
        make_settings_menu(mainwindow, menubar, accelgroup);
        make_search_menu(mainwindow, menubar, accelgroup);

        make_toolbar(mainwindow, vbox);

        make_editor(vbox);

        static nu::statusbar_t status_bar(vbox);
        _statusbar_ptr = &status_bar;

        static nu::statusbar_t error_bar(vbox);
        _errorbar_ptr = &error_bar;

        vbox.on_delete_event(exit_main_window);

        configure_editor();

        nu::widget_t<nu::window_t>(mainwindow).show_all();
        nu::widget_t<nu::editor_t>(editor()).grab_focus();

        editor().set_notify_cbk(on_scintilla_notification);

        set_default_icon();

        mainwin().show();
        errorbar().hide();

        gtk_widget_queue_draw (mainwin().get_internal_obj());
    }


    /* ---------------------------------------------------------------------- */

    const nu::editor_t& editor() const noexcept {
        assert(_editor_ptr);
        return *_editor_ptr;
    }

    /* ---------------------------------------------------------------------- */

    nu::editor_t& editor() noexcept {
        assert(_editor_ptr);
        return *_editor_ptr;
    }


    /* ---------------------------------------------------------------------- */

    nu::window_t& mainwin() noexcept {
        assert(_mainwin_ptr);
        return *_mainwin_ptr;
    }


    /* ---------------------------------------------------------------------- */

    nu::statusbar_t& statusbar() noexcept {
        assert(_statusbar_ptr);
        return * _statusbar_ptr;
    }


    /* ---------------------------------------------------------------------- */

    nu::statusbar_t& errorbar() noexcept {
        assert(_errorbar_ptr);
        return * _errorbar_ptr;
    }


    /* ---------------------------------------------------------------------- */

    void run() const noexcept {
        gtk_main();
    }


    /* ---------------------------------------------------------------------- */

    const char* get_working_path() const noexcept {
        return _working_path.c_str();
    }


    /* ---------------------------------------------------------------------- */

    const char* get_working_file() const noexcept {
        return _working_file.c_str();
    }


    /* ---------------------------------------------------------------------- */

    void set_working_path(const char* working_path) noexcept {
        _working_path = working_path;
    }


    /* ---------------------------------------------------------------------- */

    void set_working_file(const char* working_file) noexcept {
        _working_file = working_file;
    }


    /* ---------------------------------------------------------------------- */

    static app_t& get_instance() noexcept {
        assert(_this);
        return *_this;
    }


    /* ---------------------------------------------------------------------- */

    void set_dirty_flg(bool state = true) noexcept {
        _is_dirty = state;
    }


    /* ---------------------------------------------------------------------- */

    bool is_dirty() const noexcept {
        return _is_dirty;
    }


    /* ---------------------------------------------------------------------- */

    void set_needbuild_flg(bool state = true) noexcept {
        _need_build = state;
    }


    /* ---------------------------------------------------------------------- */

    std::string cfg(const std::string& key) const noexcept {
        std::string value;

        auto it = _cfg.find(key);
        if (it !=_cfg.end()) {
            value = it->second;
        }

        return value;
    }


    /* ---------------------------------------------------------------------- */

    void set_cfg(const std::string& key, const std::string& value) noexcept {
        _cfg[key] = value;
    }


    /* ---------------------------------------------------------------------- */

    void remove_prog_cnt_marker() {
        auto & ed = editor();

        ed.cmd(SCI_MARKERDELETEALL, int(marker_t::PROGCOUNTER), 0);
        ed.cmd(SCI_MARKERDELETEALL, int(marker_t::LINESELECTION), 0);
        ed.cmd(SCI_LINESCROLLDOWN, 0, 0);
        ed.cmd(SCI_LINESCROLLUP, 0, 0);
    }


    /* ---------------------------------------------------------------------- */

    bool has_bookmark(long line) const noexcept {
        const auto m = int(marker_t::BOOKMARK) + 1;
        return ((editor().cmd(SCI_MARKERGET, line - 1, 0) & m) == m);
    }


    /* ---------------------------------------------------------------------- */

    bool has_breakpoint(long line) const noexcept {
        const auto m = int(marker_t::BREAKPOINT) + 1;
        return ((editor().cmd(SCI_MARKERGET, line - 1, 0) & m) == m);
    }


    /* ---------------------------------------------------------------------- */

    void remove_all_breakpoints() noexcept {
        editor().cmd(SCI_MARKERDELETEALL, int(marker_t::BREAKPOINT), 0);
    }


    /* ---------------------------------------------------------------------- */

    void reset_all_breakpoints() noexcept {
        auto linecount = editor().get_line_count();

        interpreter().exec_command("clrbrk");

        for (int i = 0; i < linecount; ++i)
            if (has_breakpoint(i + 1))
                interpreter().exec_command("break " + std::to_string(i + 1));
    }

    /* -------------------------------------------------------------------------- */

    bool show_execution_point(int line) noexcept {
        remove_prog_cnt_marker();

        if (line < 1)
            line = 1;

        auto & ed = editor();

        auto endpos = ed.cmd(SCI_GETLINEENDPOSITION, line - 1, 0) + 1;

        while (!interpreter().has_runnable_stmt(line) && line < endpos)
            ++line;

        if (ed.cmd(SCI_POSITIONFROMLINE, line - 1, 0) >= 0) {
            const auto l = int(marker_t::LINESELECTION);

            ed.cmd(SCI_MARKERDEFINE, l, SC_MARK_BACKGROUND);
            ed.cmd(SCI_MARKERSETBACK, l, RGB(0, 255, 0));
            ed.cmd(SCI_MARKERADD, line - 1, l);


            const auto m = int(marker_t::PROGCOUNTER);

            ed.cmd(SCI_MARKERDEFINE, m, SC_MARK_ARROW);
            ed.cmd(SCI_MARKERSETFORE, m, RGB(0, 0, 0));
            ed.cmd(SCI_MARKERSETBACK, m, RGB(0, 0, 255));

            ed.cmd(SCI_MARKERADD, line - 1, m);

            ed.go_to_line(line);

            ed.update_ui();

        } else {
            ed.cmd(SCI_MARKERDELETE, line - 1, int(marker_t::LINESELECTION));
            ed.cmd(SCI_MARKERDELETE, line - 1, int(marker_t::PROGCOUNTER));
        }

        return true;
    }


    /* ---------------------------------------------------------------------- */

    bool show_error_line(int line) noexcept {
        remove_prog_cnt_marker();
        auto & ed = editor();

        if (line < 1)
            line = 1;

        if (ed.cmd(SCI_POSITIONFROMLINE, line - 1, 0) >= 0) {
            const auto l = int(marker_t::LINESELECTION);

            ed.cmd(SCI_MARKERDEFINE, l, SC_MARK_BACKGROUND);
            ed.cmd(SCI_MARKERSETBACK, l, RGB(255, 0, 0));
            ed.cmd(SCI_MARKERADD, line - 1, l);

            ed.go_to_line(line);

            ed.update_ui();
        } else {
            ed.cmd(SCI_MARKERDELETE, line - 1, int(marker_t::LINESELECTION));
        }

        return true;
    }


    /* -------------------------------------------------------------------------- */

    bool exec_interpreter_cmd(const std::string& cmd, bool bg_mode)
    {
        auto async_fn = [&]() {

            try {
                interpreter().exec_command(cmd);
                // RunningDlgWndProc_terminate = true;
                return true;
            } catch (nu::runtime_error_t& e) {
                char buf[2048] = { 0 };
                int line = e.get_line_num();
                line = line <= 0 ? interpreter().get_cur_line_n() : line;

                snprintf(buf, sizeof(buf) - 1, "Runtime Error #%i at %i %s\n",
                        e.get_error_code(), line, e.what());

                // add_info(buf, CFM_BOLD | CFM_ITALIC);
                nu::msgbox(mainwin(), buf, "Runtime Error");

            } catch (std::exception& e) {
                char buf[2048] = { 0 };

                if (interpreter().get_cur_line_n() > 0)
                    snprintf(buf, sizeof(buf) - 1, "At line %i: %s\n",
                            interpreter().get_cur_line_n(), e.what());
                else
                    snprintf(buf, sizeof(buf) - 1, "%s\n", e.what());

                nu::msgbox(mainwin(), buf, "Runtime Error");
                //g_editor.add_info(buf, CFM_BOLD | CFM_ITALIC);
            }

            // RunningDlgWndProc_terminate = true;
            return false;
        };

        if (bg_mode) {
            std::thread t(async_fn);

            //add_info("Run Program\n", CFM_ITALIC);

            //show_running_dialog(); // TODO

            interpreter().set_step_mode(true);

            t.join();

            interpreter().set_step_mode(false);

            // add_info("Stop Program\n", CFM_ITALIC);

            return true;
        }

        return async_fn();
    }


    /* -------------------------------------------------------------------------- */

    void start_debugging(dbg_flg_t flg = dbg_flg_t::NORMAL_EXECUTION) {

        if (_need_build && !rebuild_code(true))
            return;

        reset_all_breakpoints();
        remove_prog_cnt_marker();

        // alloc_console(); // TODO

        interpreter().register_break_event();

        switch (flg) {
            case dbg_flg_t::NORMAL_EXECUTION:
                exec_interpreter_cmd("run", /* true */ false);
                break;
            case dbg_flg_t::CONTINUE_EXECUTION:
                exec_interpreter_cmd("cont", /* true */ false);
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

    void continue_debugging() {
        start_debugging(dbg_flg_t::CONTINUE_EXECUTION);
    }


    /* -------------------------------------------------------------------------- */

    void singlestep_debugging() {
        start_debugging(dbg_flg_t::SINGLE_STEP_EXECUTION);
    }


    /* -------------------------------------------------------------------------- */

    bool rebuild_code(bool show_err_msg) noexcept {
        remove_prog_cnt_marker();

        auto & ed = editor();

        auto doc_size = ed.cmd(SCI_GETLENGTH);

        if (doc_size <= 0)
            return true;

#if 0
        RECT rcClient; // Client area of parent window.
        GetClientRect(_h_splitter, &rcClient);

        int cyVScroll = GetSystemMetrics(SM_CYVSCROLL);

        HWND hwndPB
            = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE,
                    rcClient.left, rcClient.bottom - cyVScroll, rcClient.right,
                    cyVScroll, _h_splitter, (HMENU)0, get_instance_handle(), NULL);
#endif


        int cb = int(doc_size / 10);

#if 0
        // Set the range and increment of the progress bar.
        SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, cb));
        SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)1, 0);
#endif

        std::vector<char> data(doc_size + 1);
        ed.get_text_range(0, int(doc_size), data.data());

        decltype(doc_size) i = 0;
        std::string line;
        int line_num = 1;
        interpreter().clear_all();
        bool old_style_prog = false;

        // remove_funcs_menu(); // TODO

        bool first_is_special_comment = false;

        while (i < doc_size) {
            // if ((i % 10) == 0)
                //SendMessage(hwndPB, PBM_STEPIT, 0, 0);

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

                mainwin().set_title(line.c_str());

                if (!res) {
                    // DestroyWindow(hwndPB);
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
        // create_funcs_menu(); // TODO
        // DestroyWindow(hwndPB);

        _need_build = false;

        nu::msgbox(mainwin(), "Build succeeded.", "Build");
        errorbar().set_text("");
        errorbar().hide();

        return true;
    }




    /* -------------------------------------------------------------------------- */

    bool build_basic_line(
        const std::string& line, int line_num, bool dump_err_msg)
    {
        try {
            // Ignore first line if it begins with #!
            if (line_num == 1 && line.size() > 2 && line[0] == '#'
                    && line[1] == '!')
                return true;

            if (!interpreter().update_program(line, line_num)) {
                std::string msg = "Syntax Error at line ";
                msg += line_num ? std::to_string(line_num) : "\r\n";
                msg += line_num ? "" : line;

                show_error_line(line_num);

                if (dump_err_msg) {
                    // g_editor.add_info("\n" + msg + "\n", CFM_BOLD | CFM_ITALIC);
                    errorbar().show();
                    errorbar().set_text(msg.c_str(), GTK_MESSAGE_INFO);
                    nu::msgbox(mainwin(), msg.c_str(), "Syntax Error");
                }

                return false;
            }
        }
        // Print out Runtime Error Messages
        catch (nu::runtime_error_t& e) {
            if (!dump_err_msg)
                return false;

            int line = e.get_line_num();
            line = line <= 0 ? interpreter().get_cur_line_n() : line;

            char lbuf[2048] = { 0 };

            snprintf(lbuf, sizeof(lbuf) - 1, "Error #%i at %i %s\n",
                    e.get_error_code(), line, e.what());

            show_error_line(line);
            // g_editor.add_info(lbuf, CFM_BOLD | CFM_ITALIC);

            errorbar().set_text(lbuf, GTK_MESSAGE_INFO);
            errorbar().show();
            nu::msgbox(mainwin(), lbuf, "Syntax Error");

            return false;
        }
        // Print out Syntax Error Messages
        catch (std::exception& e) {
            if (!dump_err_msg)
                return false;

            char lbuf[2048] = { 0 };

            const auto line = interpreter().get_cur_line_n();

            if (line > 0) {
                snprintf(
                        lbuf, sizeof(lbuf) - 1, "At line %i: %s\n", line, e.what());

                show_error_line(line);
            } else {
                snprintf(lbuf, sizeof(lbuf) - 1, "%s\n", e.what());
            }

            //g_editor.add_info(lbuf, CFM_BOLD | CFM_ITALIC);

            errorbar().set_text(lbuf, GTK_MESSAGE_INFO);
            errorbar().show();
            nu::msgbox(mainwin(), lbuf, "Error");

            return false;
        }

        return true;
    }


    nu::interpreter_t& interpreter() noexcept { 
        return _interpreter; 
    }

private:
    nu::interpreter_t _interpreter;

    nu::editor_t * _editor_ptr = nullptr;
    nu::window_t * _mainwin_ptr = nullptr;
    nu::statusbar_t * _statusbar_ptr = nullptr;
    nu::statusbar_t * _errorbar_ptr = nullptr;

    std::string _working_file;
    std::string _working_path;

    std::string _full_path_str;

    bool _is_dirty = false;
    bool _need_build = false;

    std::unordered_map<std::string, std::string> _cfg;


    static app_t * _this;
};


/* -------------------------------------------------------------------------- */

app_t * app_t::_this = nullptr;


/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

    nu::create_terminal_frame(argc, argv);

    app_t app(argc, argv);
    app.run();

    return (0);
}

