#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "include/gvimsurfer2.h"
#include "include/client.h"
#include "include/callbacks.h"
#include "include/completion.h"

// --- local function declarations ---
GtkWidget * create_notebook_label(const gchar*, GtkWidget*, gint);
void        init_client_data();

void init_client() {

   GdkDisplay *display = gdk_display_get_default ();
   GdkScreen *screen = gdk_display_get_default_screen (display);

   //--- global init ---
   Client.keymap  = gdk_keymap_get_default();

   //--- init UI ---
   Client.UI.window           = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   
   // set window size
   gtk_window_set_default_size(GTK_WINDOW(Client.UI.window), default_width, default_height);

   // setup signals
   g_signal_connect(Client.UI.window, "destroy",   G_CALLBACK(cb_destroy), NULL);

   //--- set up CSS UI provider ---
   Client.UI.style_provider   = gtk_css_provider_new();
   GError *error = 0;
   gtk_css_provider_load_from_file(Client.UI.style_provider, g_file_new_for_path(ui_cssfile), &error);

   gtk_style_context_add_provider_for_screen (
      screen, 
      GTK_STYLE_PROVIDER (Client.UI.style_provider), 
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
   );
   g_object_unref(Client.UI.style_provider);

   //--- notebook ---
   Client.UI.notebook   = GTK_NOTEBOOK(gtk_notebook_new());

   gtk_notebook_set_show_tabs(Client.UI.notebook,   TRUE);
   gtk_notebook_set_scrollable(Client.UI.notebook,  TRUE);
   gtk_notebook_set_show_border(Client.UI.notebook, FALSE);
   
   g_signal_connect(Client.UI.notebook, "switch-page",      G_CALLBACK(cb_notebook_switch_page), NULL);
   g_signal_connect(Client.UI.notebook, "key-press-event",  G_CALLBACK(cb_notebook_keypress), NULL);

   //--- status bar ---
   Client.UI.statusbar        = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
   gtk_widget_set_name(GTK_WIDGET(Client.UI.statusbar), "statusbar");

   Client.Statusbar.message   = GTK_LABEL(gtk_label_new(NULL));
   Client.Statusbar.uri       = GTK_LABEL(gtk_label_new(NULL));
   Client.Statusbar.info      = GTK_LABEL(gtk_label_new(NULL));
   gtk_widget_set_name(GTK_WIDGET(Client.Statusbar.uri), "statusbar");

   gtk_label_set_xalign(Client.Statusbar.message, 0.0);
   gtk_label_set_xalign(Client.Statusbar.uri, 1.0);
   gtk_label_set_xalign(Client.Statusbar.info, 1.0);

   gtk_box_pack_start(Client.UI.statusbar, GTK_WIDGET(Client.Statusbar.message), FALSE, TRUE, 0);
   gtk_box_pack_start(Client.UI.statusbar, GTK_WIDGET(Client.Statusbar.uri), TRUE, TRUE, 0);
   gtk_box_pack_start(Client.UI.statusbar, GTK_WIDGET(Client.Statusbar.info), FALSE, FALSE, 0);

   //--- input bar ---
   Client.UI.inputbar         = GTK_ENTRY(gtk_entry_new());
   gtk_widget_set_name(GTK_WIDGET(Client.UI.inputbar), "inputbar");

   gtk_entry_set_has_frame(Client.UI.inputbar, FALSE);
   gtk_editable_set_editable(GTK_EDITABLE(Client.UI.inputbar), TRUE);

   g_signal_connect(Client.UI.inputbar, "key-press-event",  G_CALLBACK(cb_inputbar_keypress), NULL);
   g_signal_connect(Client.UI.inputbar, "activate",         G_CALLBACK(cb_inputbar_activate), NULL);

   //--- packing ---
   Client.UI.box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
   gtk_container_add(GTK_CONTAINER(Client.UI.window), GTK_WIDGET(Client.UI.box));
   gtk_box_pack_start(GTK_BOX(Client.UI.box), GTK_WIDGET(Client.UI.notebook), TRUE, TRUE, 0);
   gtk_box_pack_end(GTK_BOX(Client.UI.box), GTK_WIDGET(Client.UI.inputbar), FALSE, FALSE, 0);
   gtk_box_pack_end(GTK_BOX(Client.UI.box), GTK_WIDGET(Client.UI.statusbar), FALSE, FALSE, 0);

   init_client_data();
}

void init_client_data() {
   // read bookmarks
   if(g_file_test(bookmarks, G_FILE_TEST_IS_REGULAR)){
      gchar* content = NULL;

      if(g_file_get_contents(bookmarks, &content, NULL, NULL)) {
         gchar **lines = g_strsplit(content, "\n", -1);
         gint    n     = g_strv_length(lines) - 1;

         for(gint i = 0; i < n; i++) {
            if(!strlen(lines[i]))    continue;
            
            BMark* bmark = malloc(sizeof(BMark));
            bmark->uri = g_strdup(strtok(lines[i], " "));
            bmark->tags = g_strdup(strtok(NULL, "\n"));

            Client.bookmarks = g_list_append(Client.bookmarks, bmark);
         }
         g_free(content);
         g_strfreev(lines);
      }
   }

   // read history
   if(g_file_test(history, G_FILE_TEST_IS_REGULAR)) {
      gchar* content = NULL;

      if(g_file_get_contents(history, &content, NULL, NULL)) {
         gchar **lines = g_strsplit(content, "\n", -1);
         gint    n     = g_strv_length(lines) - 1;

         for(gint i = 0; i < n; i++) {
            if(!strlen(lines[i]))   continue;

            Client.history = g_list_prepend(Client.history, g_strdup(g_strstrip(lines[i])));
         }
         Client.history = g_list_reverse(Client.history);

         g_free(content);
         g_strfreev(lines);
      }
   }
}

GtkWidget* create_tab(const gchar* uri, gboolean background) {
   say(DEBUG, "create_tab", -1);
   if (!uri) uri = home_page;

   WebKitWebView *wv = (WebKitWebView*)webkit_web_view_new();

   int ntabs      = gtk_notebook_get_n_pages(Client.UI.notebook);
   int position   = ntabs;

   GtkWidget *label = create_notebook_label("", GTK_WIDGET(Client.UI.notebook), position);
   gtk_notebook_insert_page(Client.UI.notebook, GTK_WIDGET(wv), label, position);

   //--- connect webview callbacks ---
   g_object_connect(G_OBJECT(wv),
      "signal::load-changed",          G_CALLBACK(cb_wv_load_status),   NULL,
      NULL);

   // open URI
   open_uri(WEBKIT_WEB_VIEW(wv), uri);
   change_mode(NORMAL);

   return GTK_WIDGET(wv);
}

GtkWidget* create_notebook_label(const gchar *text, GtkWidget *notebook, gint page) {
   
   GtkWidget *image;
   GtkWidget *hbox     = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
   GtkWidget *label    = gtk_label_new(text);
   GtkWidget *button   = gtk_event_box_new();

   g_object_set_data( G_OBJECT(button), "page", GINT_TO_POINTER(page) );
   g_signal_connect(button, "button_press_event", G_CALLBACK(cb_button_close_tab), GTK_NOTEBOOK(notebook) );

   gtk_box_pack_start(GTK_BOX(hbox),   label,  TRUE,  TRUE,    0);
   gtk_box_pack_end(GTK_BOX(hbox),     button, FALSE, FALSE,   0);

   image = gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_MENU);
   gtk_event_box_set_visible_window(GTK_EVENT_BOX(button), FALSE);
   gtk_container_add (GTK_CONTAINER(button), image);

   gtk_widget_show_all( hbox );

   return( hbox );
}

void close_tab(gint tab_id) {

   //Page* page = get_current_page();
   //for(GList* list = page->pagemarks; list; list = g_list_next(list))
   //   free(list->data);
   //g_list_free(page->pagemarks);

   //Client.Global.pages = g_list_remove(Client.Global.pages, page);
   //g_free(page);

   //Client.Global.last_closed = g_strdup((gchar *) webkit_web_view_get_uri(GET_CURRENT_TAB()));

   // Note: Last tab is always "addtab" button
   if (gtk_notebook_get_n_pages(Client.UI.notebook) > 1) {
      gtk_notebook_remove_page(Client.UI.notebook, tab_id);
      //if(tab_id == gtk_notebook_get_n_pages(Client.UI.notebook)-1)
      //   gtk_notebook_set_current_page(Client.UI.notebook, tab_id-1);

      update_client(gtk_notebook_get_current_page(Client.UI.notebook));
   } else 
      cb_destroy(NULL, NULL);
}

void update_client(gint tab_id){
   say(DEBUG, "update_client", -1);

   if(!Client.UI.notebook || !gtk_notebook_get_n_pages(Client.UI.notebook)) return;

   //--- update tabbar -----
   int number_of_tabs  = gtk_notebook_get_n_pages(Client.UI.notebook);
   for(int tc = 0; tc < number_of_tabs; tc++) {
      GtkWidget* this_tab     = GTK_WIDGET(gtk_notebook_get_nth_page(Client.UI.notebook, tc));
      WebKitWebView* this_wv  = (WebKitWebView*)gtk_notebook_get_nth_page(Client.UI.notebook, tc);

      const gchar* tab_title = webkit_web_view_get_title(this_wv);
      int progress = webkit_web_view_get_estimated_load_progress(this_wv) * 100;
      gchar* n_tab_title = g_strdup_printf("%d: %s", tc + 1, tab_title ? tab_title : ((progress == 100) ? "Loading..." : "(Untitled)"));
      
      // shorten title if needed
      gchar* new_tab_title = shorten_text(n_tab_title, max_title_length);

      GtkWidget* label = create_notebook_label(new_tab_title, GTK_WIDGET(Client.UI.notebook), tc);
      gtk_notebook_set_tab_label(Client.UI.notebook, this_tab, label);

      // If current tab, update title and status bar info
      if (tc == tab_id){
         update_statusbar_info(tab_id);

         //--- update uri -----
         const gchar* uri = webkit_web_view_get_uri(this_wv);
         gchar* n_uri = g_strdup_printf("%s", uri);

         // check for https
         gboolean ssl = uri ? g_str_has_prefix(uri, "https://") : FALSE;
         
         ssl ?
            gtk_widget_set_state_flags (GTK_WIDGET(Client.Statusbar.uri), GTK_STATE_FLAG_CHECKED, TRUE) :
            gtk_widget_set_state_flags (GTK_WIDGET(Client.Statusbar.uri), GTK_STATE_FLAG_NORMAL, TRUE); 

         // shorten uri if needed
         gchar* new_uri = shorten_text(n_uri, max_url_length);

         gtk_label_set_text((GtkLabel*) Client.Statusbar.uri, new_uri);
         g_free(n_uri);
         g_free(new_uri);
         
         //--- update title -----
         const gchar* web_title = webkit_web_view_get_title(this_wv);
         gchar* title = g_strdup_printf("%s %s: %s", NAME, private_mode ? "(PRIVATE)" : "", web_title);
         if(title)    gtk_window_set_title(GTK_WINDOW(Client.UI.window), title);

         g_free(title);
      }
      g_free(n_tab_title);
      g_free(new_tab_title);
   }

   gtk_widget_show_all(GTK_WIDGET(Client.UI.notebook));
}

void update_statusbar_info(gint tab_id) {
   WebKitWebView* this_wv  = (WebKitWebView*)gtk_notebook_get_nth_page(Client.UI.notebook, tab_id);

   GString *status = g_string_new("");

   // update page load and download status
   {
      // check for possible navigation
      GString* navigation = g_string_new("");
      g_string_append_c(navigation, webkit_web_view_can_go_back(this_wv)?'<':'-');
      g_string_append_c(navigation, webkit_web_view_can_go_forward(this_wv)?'>':'-');

      g_string_append_printf(status, " [%s]\t", navigation->str);
      g_string_free(navigation, TRUE);

      //page load status
      gint progress = -1;
      progress = webkit_web_view_get_estimated_load_progress(this_wv)*100;
      if(progress>0 && progress<100)
         g_string_append_printf(status, "[%d%%]\t", progress);
   
      /*
      // download status
      if(Client.Global.active_downloads){
         gint download_count = g_list_length(Client.Global.active_downloads);

         progress=0;
         for(GList *ptr = Client.Global.active_downloads; ptr; ptr= g_list_next(ptr))
            progress += 100*webkit_download_get_progress(ptr->data);

         progress /= download_count;

         g_string_append_printf(status, "[%d DL: %d%%]\t", download_count, progress);
      }
      */
   }

   /*
   // update position
   GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(this_tab);
   gdouble view_size         = gtk_adjustment_get_page_size(adjustment);
   gdouble value             = gtk_adjustment_get_value(adjustment);
   gdouble max               = gtk_adjustment_get_upper(adjustment) - view_size;

   gchar* position;
   if(max == 0)            position = g_strdup("All");
   else if(value == max)   position = g_strdup("Bot");
   else if(value == 0)     position = g_strdup("Top");
   else
      position = g_strdup_printf("%2d%%", (int) ceil(((value / max) * 100)));

   g_string_append_printf(status, "%s", position);
   */
   gtk_label_set_text((GtkLabel*) Client.Statusbar.info, status->str);

   g_string_free(status, TRUE);
   //g_free(position);
}

void clear_inputbar() {
   run_completion(HIDE);

   //gtk_label_set_text((GtkLabel*) Client.Statusbar.message, "");
   gtk_entry_set_text(Client.UI.inputbar, "");
   change_mode(NORMAL);
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_PAGE()));
   set_inputbar_visibility(HIDE);
}

void set_inputbar_visibility(gint visibility) {
   say(DEBUG, "set_inputbar_visibility", -1);
   gboolean is_visible = gtk_widget_get_visible(GTK_WIDGET(Client.UI.inputbar));

   if(visibility==HIDE || (visibility==TOGGLE && is_visible)){
      gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));
      //if(show_statusbar)
         gtk_widget_show(GTK_WIDGET(Client.UI.statusbar));
      //else
      //   gtk_widget_hide(GTK_WIDGET(Client.UI.statusbar));
   }else if(visibility==SHOW || (visibility==TOGGLE && !is_visible)){
      gtk_widget_show(GTK_WIDGET(Client.UI.inputbar));
      gtk_widget_hide(GTK_WIDGET(Client.UI.statusbar));
   }

}
