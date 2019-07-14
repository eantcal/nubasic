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
            const char* title = _replace_controls ?
                "Replace" : "Find";

            _window = gtk_dialog_new_with_buttons(title,
                    GTK_WINDOW (parent.get_internal_obj()),
                    GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_STOCK_CLOSE,
                    GTK_RESPONSE_NONE,
                    NULL);

            gtk_window_set_resizable(GTK_WINDOW(_window), TRUE);

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
            gtk_label_set_markup(GTK_LABEL(label), "Find what:");
            gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

            auto hbox = gtk_hbox_new(FALSE, 10);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
            gtk_container_set_border_width(GTK_CONTAINER (hbox), 0);

            auto entry = gtk_entry_new();
            gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

            if (_replace_controls) {
                _replace_entry = gtk_entry_new();
                gtk_entry_set_max_length(GTK_ENTRY(_replace_entry),0);
                gtk_entry_set_text(GTK_ENTRY(_replace_entry), _replace_text.c_str());

                auto label = gtk_label_new_with_mnemonic ("_Replace with:");

                gtk_container_add(GTK_CONTAINER(vbox), label);
                gtk_container_add(GTK_CONTAINER(vbox), _replace_entry);
            }

            gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

            auto find_button = gtk_button_new_with_label("Find Next");
            g_signal_connect(find_button, "clicked", G_CALLBACK (find), entry);
            gtk_widget_show(find_button);
            gtk_container_add(GTK_CONTAINER(vbox), find_button);

            if (_replace_controls) {
                auto replace_button = gtk_button_new_with_label("Replace");
                g_signal_connect(replace_button, "clicked", G_CALLBACK(replace), entry);
                gtk_widget_show(replace_button);
                gtk_container_add(GTK_CONTAINER(vbox), replace_button);

                replace_button = gtk_button_new_with_label("Replace All");
                g_signal_connect(replace_button, "clicked", G_CALLBACK(replace_all), entry);
                gtk_widget_show(replace_button);
                gtk_container_add(GTK_CONTAINER(vbox), replace_button);
            }


            _match_word_only = gtk_check_button_new_with_mnemonic ("Match _Word only");
            _match_case = gtk_check_button_new_with_mnemonic ("Match _Case");

            gtk_container_add(GTK_CONTAINER(vbox), _match_word_only);
            gtk_container_add(GTK_CONTAINER(vbox), _match_case);

            search_text(NULL, GTK_ENTRY(entry));

            gtk_entry_set_icon_from_stock(
                GTK_ENTRY(entry),
                GTK_ENTRY_ICON_SECONDARY,
                GTK_STOCK_CLEAR);

            g_signal_connect(entry, "icon-press", G_CALLBACK(icon_press_cb), NULL);

            gtk_entry_set_text(GTK_ENTRY(entry), _find_text.c_str());

            _menu = create_search_menu(entry);
            gtk_menu_attach_to_widget(GTK_MENU(_menu), entry, NULL);
            g_signal_connect(entry, "populate-popup",G_CALLBACK (entry_populate_popup), NULL);

            // Create an initial radio button
            _btn_forward = gtk_radio_button_new_with_label (NULL, "Forward");

            // Create a second radio button, and add it to the same group as _btn_forward
            _btn_backward = 
                gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(_btn_forward), 
                    "Backward");

            gtk_container_add(GTK_CONTAINER(vbox), _btn_forward);
            gtk_container_add(GTK_CONTAINER(vbox), _btn_backward);

            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_btn_forward), TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_btn_backward), FALSE);
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

    const std::string& get_text() const noexcept {
        return _find_text;
    }

   
    /* ---------------------------------------------------------------------- */

    const std::string& get_replace_text() const noexcept {
        return _replace_text;
    }


    /* ---------------------------------------------------------------------- */

    struct observer_t {
        virtual void notify_find_result( dialog_search_t & ctx ) noexcept {
            (void) ctx;
        }

        virtual void notify_replace_result( dialog_search_t & ctx ) noexcept {
            (void) ctx;
        }

        virtual void notify_replace_all_result( dialog_search_t & ctx ) noexcept {
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

    static gboolean search_progress (gpointer data) {
        gtk_entry_progress_pulse(GTK_ENTRY (data));
        return TRUE;
    }


    /* ---------------------------------------------------------------------- */

    enum class op_t { FIND, REPLACE, REPLACE_ALL };

    static void find_and_replace(GtkButton *button, GtkEntry *entry, op_t op) {
        auto & _this = get_instance();

        auto forward = 
            GTK_TOGGLE_BUTTON (_this._btn_forward)->active;

        _this.set_text(gtk_entry_get_text(entry));

        if (op != op_t::FIND) {
            auto replace_str = 
                gtk_entry_get_text(GTK_ENTRY(_this._replace_entry));

            assert( replace_str );

            _this.set_replace_text( replace_str );
        }

        _this._search_direction_forward = forward;
        auto observer = _this._observer;

        auto match_word_only = 
            GTK_TOGGLE_BUTTON (_this._match_word_only)->active;

        auto match_case = 
            GTK_TOGGLE_BUTTON (_this._match_case)->active;

        if (match_case) {
            _this._flgs |= MATCHCASE;
        }
        else {
            _this._flgs &= ~MATCHCASE;
        }

        if (match_word_only) {
            _this._flgs |= WHOLEWORD;
        }
        else {
            _this._flgs &= ~WHOLEWORD;
        }

        if (observer) {
            switch( op ) {
                case op_t::FIND:
                    observer->notify_find_result( _this );
                    break;

                case op_t::REPLACE:
                    observer->notify_replace_result( _this );
                    break;

                case op_t::REPLACE_ALL:
                    observer->notify_replace_all_result( _this );
                    break;
            }
        }
    }


    /* ---------------------------------------------------------------------- */

    static void find(GtkButton *button, GtkEntry  *entry) {
        find_and_replace( button, entry, op_t::FIND );
    }


    /* ---------------------------------------------------------------------- */

    static void replace(GtkButton *button, GtkEntry  *entry) {
        find_and_replace( button, entry, op_t::REPLACE );
    }


    /* ---------------------------------------------------------------------- */

    static void replace_all(GtkButton *button, GtkEntry  *entry) {
        find_and_replace( button, entry, op_t::REPLACE_ALL );
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
        _find_text = text;
    }


    /* ---------------------------------------------------------------------- */

    void set_replace_text(const char* text) noexcept {
        _replace_text = text;
    }


    /* ---------------------------------------------------------------------- */

    int get_flags() const noexcept {
        return _flgs;
    }


    /* ---------------------------------------------------------------------- */

    bool is_forward() const noexcept {
        return _search_direction_forward;
    }


    /* ---------------------------------------------------------------------- */

    bool set_replace_mode(bool on) noexcept {
        _replace_controls = on;
	return true;
    }

private:
    observer_t * _observer = nullptr;
    std::string _find_text;
    std::string _replace_text;

    GtkWidget *_window = nullptr;
    GtkWidget *_menu = nullptr;
    GtkWidget *_match_word_only = nullptr;
    GtkWidget *_match_case = nullptr;
    GtkWidget *_btn_forward = nullptr;
    GtkWidget *_btn_backward = nullptr;
    GtkWidget *_replace_entry = nullptr;

    int _flgs = 0;
    bool _search_direction_forward = true;

    bool _replace_controls = false;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_DIALOG_SEARCH_H__ 

