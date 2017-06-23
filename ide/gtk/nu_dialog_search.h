//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_DIALOG_SEARCH_H__
#define __NU_DIALOG_SEARCH_H__


/* -------------------------------------------------------------------------- */

#include "nu_window.h"

#include <gtk/gtk.h>
#include <cassert>

#include <mutex>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class dialog_search_t
{
public:
    dialog_search_t(const dialog_search_t&) = delete;
    dialog_search_t& operator=(const dialog_search_t&) = delete;

    enum { MATCHCASE = 1, WHOLEWORD = 2 };
            

    /* ---------------------------------------------------------------------- */

    void run(const window_t& parent) noexcept {
        if (!_window) {
            _window = gtk_dialog_new_with_buttons("Find...",
                    GTK_WINDOW (parent.get_internal_obj()),
                    GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_STOCK_CLOSE,
                    GTK_RESPONSE_NONE,
                    NULL);

            gtk_window_set_resizable(GTK_WINDOW(_window), FALSE);

            g_signal_connect(_window, "response",
                G_CALLBACK(gtk_widget_destroy), NULL);

            g_signal_connect(_window, "destroy",
                G_CALLBACK(search_entry_destroyed), &_window);

            auto vbox = gtk_vbox_new(FALSE, 5);

            gtk_box_pack_start(
                GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(_window))), 
                vbox, 
                TRUE, 
                TRUE, 
                0
            );

            gtk_container_set_border_width(GTK_CONTAINER (vbox), 5);

            auto label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(label), "Find what");
            gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

            auto hbox = gtk_hbox_new(FALSE, 10);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
            gtk_container_set_border_width(GTK_CONTAINER (hbox), 0);

            auto entry = gtk_entry_new();
            gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);

            _notebook = gtk_notebook_new();
            gtk_notebook_set_show_tabs(GTK_NOTEBOOK(_notebook), TRUE);
            gtk_notebook_set_show_border(GTK_NOTEBOOK(_notebook), TRUE);

            gtk_box_pack_start(GTK_BOX(hbox), _notebook, FALSE, FALSE, 0);

            auto find_down_button = gtk_button_new_with_label("Find");
            g_signal_connect(find_down_button, "clicked", G_CALLBACK (find_down), entry);

            gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), find_down_button, NULL);
            gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(_notebook), find_down_button, "Down");
            gtk_widget_show(find_down_button);

            auto find_up_button = gtk_button_new_with_label("Find");
            g_signal_connect(find_up_button, "clicked", G_CALLBACK (find_up), entry);

            gtk_notebook_append_page(GTK_NOTEBOOK(_notebook), find_up_button, NULL);
            gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(_notebook), find_up_button, "Up");
            gtk_widget_show(find_up_button);

            search_text(NULL, GTK_ENTRY(entry));

            gtk_entry_set_icon_from_stock(GTK_ENTRY(entry),
                    GTK_ENTRY_ICON_SECONDARY,
                    GTK_STOCK_CLEAR);

            g_signal_connect(entry, "icon-press", G_CALLBACK(icon_press_cb), NULL);
            // g_signal_connect(entry, "activate", G_CALLBACK(activate_cb), NULL);

            gtk_entry_set_text(GTK_ENTRY(entry), _text.c_str());

            _menu = create_search_menu(entry);
            gtk_menu_attach_to_widget(GTK_MENU(_menu), entry, NULL);
            g_signal_connect(entry, "populate-popup",G_CALLBACK (entry_populate_popup), NULL);
        }

        if (!gtk_widget_get_visible(_window)) {
            gtk_widget_show_all(_window);
        }
        else {
            gtk_widget_destroy(_menu);
            gtk_widget_destroy(_window);
            _window = nullptr;
        }
    }

    
    /* ---------------------------------------------------------------------- */

    std::string get_text() const noexcept {
        std::string ret;
        ret = _text;
        
        return ret;
    }


    /* ---------------------------------------------------------------------- */

    struct observer_t {
        virtual void notify_find_start( dialog_search_t & ctx ) noexcept {
            (void) ctx;
        }

        virtual ~observer_t() {}
    };


    /* ---------------------------------------------------------------------- */

    void register_observer( observer_t * obj ) {
        _observer = obj;
    }


    /* ---------------------------------------------------------------------- */

    static dialog_search_t& get_instance() noexcept {
        static dialog_search_t _instance;
        return _instance;
    }

private:
    dialog_search_t() = default;


    /* ---------------------------------------------------------------------- */

    static void show_find_button() {
        gtk_notebook_set_current_page(GTK_NOTEBOOK (get_instance()._notebook), 0);
    }


    /* ---------------------------------------------------------------------- */

    static gboolean search_progress (gpointer data) {
        gtk_entry_progress_pulse(GTK_ENTRY (data));
        return TRUE;
    }


    /* ---------------------------------------------------------------------- */

    static void find(GtkButton *button, GtkEntry  *entry, bool forward) {
        auto & _this = get_instance();

        _this.set_text(gtk_entry_get_text(entry));
        _this._search_direction_forward = forward;
        auto observer = _this._observer;

        if (observer) {
            observer->notify_find_start( _this );
        }
    }


    /* ---------------------------------------------------------------------- */

    static void find_down(GtkButton *button, GtkEntry  *entry) {
        find(button, entry, true);
    }


    /* ---------------------------------------------------------------------- */

    static void find_up(GtkButton *button, GtkEntry  *entry) {
        find(button, entry, false);
    }


    /* ---------------------------------------------------------------------- */

    static void clear_entry(GtkEntry *entry) {
        gtk_entry_set_text(entry, "");
    }


    /* ---------------------------------------------------------------------- */

    static void search_text(GtkWidget *item, GtkEntry  *entry) {
        gtk_entry_set_icon_from_stock (entry,
                GTK_ENTRY_ICON_PRIMARY,
                GTK_STOCK_FIND);

        gtk_entry_set_icon_tooltip_text (entry,
                GTK_ENTRY_ICON_PRIMARY,
                "Search text\n"
                "Click here to change the search type");

        get_instance()._flgs = 0;
    }


    /* ---------------------------------------------------------------------- */

    static void search_whole_word(GtkWidget *item, GtkEntry  *entry) {
        gtk_entry_set_icon_from_stock (entry,
                GTK_ENTRY_ICON_PRIMARY,
                GTK_STOCK_UNDERLINE);

        gtk_entry_set_icon_tooltip_text (entry,
                GTK_ENTRY_ICON_PRIMARY,
                "Search whole word only\n"
                "Click here to change the search type");

        get_instance()._flgs = WHOLEWORD;
    }


    /* ---------------------------------------------------------------------- */

    static void search_matching_case(GtkWidget *item, GtkEntry  *entry) {
        gtk_entry_set_icon_from_stock (entry,
                GTK_ENTRY_ICON_PRIMARY,
                GTK_STOCK_BOLD);

        gtk_entry_set_icon_tooltip_text (entry,
                GTK_ENTRY_ICON_PRIMARY,
                "Match case\n"
                "Click here to change the search type");

        get_instance()._flgs = MATCHCASE;
    }


    /* ---------------------------------------------------------------------- */

    static void search_whole_word_matching_case(GtkWidget *item, GtkEntry  *entry) {
        gtk_entry_set_icon_from_stock(entry,
                GTK_ENTRY_ICON_PRIMARY,
                GTK_STOCK_SPELL_CHECK);

        gtk_entry_set_icon_tooltip_text(entry,
                GTK_ENTRY_ICON_PRIMARY,
                "Search whole word matching case\n"
                "Click here to change the search type");

        get_instance()._flgs = MATCHCASE | WHOLEWORD;
    }


    /* ---------------------------------------------------------------------- */

    GtkWidget * create_search_menu(GtkWidget *entry) {
        auto menu = gtk_menu_new();
        auto item = gtk_image_menu_item_new_with_mnemonic("Search _text");
        auto image = gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);

        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM (item), image);
        gtk_image_menu_item_set_always_show_image (GTK_IMAGE_MENU_ITEM (item), TRUE);

        g_signal_connect (item, "activate", G_CALLBACK(search_text), entry);

        gtk_menu_shell_append(GTK_MENU_SHELL (menu), item);


        item = gtk_image_menu_item_new_with_mnemonic("Match _whole word only");
        image = gtk_image_new_from_stock(GTK_STOCK_UNDERLINE, GTK_ICON_SIZE_MENU);
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
        gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item), TRUE);

        g_signal_connect(item, "activate", G_CALLBACK(search_whole_word), entry);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


        item = gtk_image_menu_item_new_with_mnemonic("Match _case");
        image = gtk_image_new_from_stock(GTK_STOCK_BOLD, GTK_ICON_SIZE_MENU);

        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
        gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item), TRUE);

        g_signal_connect(item, "activate", G_CALLBACK(search_matching_case), entry);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);


        item = gtk_image_menu_item_new_with_mnemonic("Search _whole word matching case");
        image = gtk_image_new_from_stock(GTK_STOCK_SPELL_CHECK, GTK_ICON_SIZE_MENU);

        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM (item), image);
        gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM (item), TRUE);

        g_signal_connect(item, "activate", G_CALLBACK(search_whole_word_matching_case), entry);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


        gtk_widget_show_all (menu);

        return menu;
    }


    /* ---------------------------------------------------------------------- */

    static void icon_press_cb (
        GtkEntry *entry, 
        gint position, 
        GdkEventButton *event, 
        gpointer data)
    {
        if (position == GTK_ENTRY_ICON_PRIMARY) {
            gtk_menu_popup (GTK_MENU (get_instance()._menu), 
                    NULL, NULL, NULL, NULL,
                    event->button, event->time);
        }
        else {
            clear_entry (entry);
        }
    }


    /* ---------------------------------------------------------------------- */

    static void text_changed_cb(GtkEntry *entry, GParamSpec *pspec, GtkWidget *button) {
        gboolean has_text;

        has_text = gtk_entry_get_text_length(entry) > 0;

        gtk_entry_set_icon_sensitive (entry,
                GTK_ENTRY_ICON_SECONDARY,
                has_text);

        gtk_widget_set_sensitive(button, has_text);
    }


    /* ---------------------------------------------------------------------- */

    static void activate_cb(GtkEntry  *entry, GtkButton *button) {
        // find_down(button, entry);
    }


    /* ---------------------------------------------------------------------- */

    static void search_entry_destroyed(GtkWidget  *widget) {
        auto & _this = get_instance();
        _this._window = nullptr;
    }


    /* ---------------------------------------------------------------------- */

    static void entry_populate_popup (
        GtkEntry *entry,
        GtkMenu  *menu,
        gpointer user_data)
    {
        gboolean has_text = gtk_entry_get_text_length (entry) > 0;

        auto item = gtk_separator_menu_item_new();
        gtk_widget_show(item);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

        item = gtk_menu_item_new_with_mnemonic("C_lear");
        gtk_widget_show(item);
        g_signal_connect_swapped (item, "activate", G_CALLBACK(clear_entry), entry);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        gtk_widget_set_sensitive(item, has_text);

        auto search_menu = get_instance().create_search_menu(GTK_WIDGET(entry));
        item = gtk_menu_item_new_with_label("Search options");
        gtk_widget_show(item);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), search_menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }


    /* ---------------------------------------------------------------------- */


public:
    void set_text(const char* text) noexcept {
        _text = text;
    }


    /* ---------------------------------------------------------------------- */

    int get_flags() const noexcept {
        return _flgs;
    }


    /* ---------------------------------------------------------------------- */

    bool is_forward() const noexcept {
        return _search_direction_forward;
    }

private:
    observer_t * _observer = nullptr;
    std::string _text;

    GtkWidget *_window = nullptr;
    GtkWidget *_menu = nullptr;
    GtkWidget *_notebook = nullptr;

    int _flgs = 0;
    bool _search_direction_forward = true;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_DIALOG_SEARCH_H__ 

