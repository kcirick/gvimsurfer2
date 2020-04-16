#include <ctype.h>
#include <string.h>
#include <regex.h>

#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "include/gvimsurfer2.h"
#include "include/client.h"
#include "include/callbacks.h"
#include "include/shortcuts.h"
#include "include/completion.h"
#include "include/commands.h"
#include "include/sc_defs.h"
#include "include/cmd_defs.h"


static gint static_page_id;

gboolean cb_blank() { return TRUE; }

gboolean cb_destroy(GtkWidget* widget, gpointer data) {
   cmd_closeall(0, NULL);
   //gtk_main_quit ();
   
   return TRUE;
}

//--- button callbacks ---
gboolean cb_button_close_tab(GtkButton *button, GtkNotebook *notebook) {
   gint page_id = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( button ), "page" ) );
   close_tab(page_id);
  
   return TRUE;
}

//--- notebook callbacks ---
gboolean cb_notebook_switch_page(GtkNotebook *notebook, gpointer page, guint page_num, gpointer user_data) {
   static_page_id = gtk_notebook_get_current_page(notebook);
   
   update_client(page_num);
   
   return TRUE;
}

gboolean cb_notebook_keypress(GtkWidget* widget, GdkEventKey* event, gpointer data) {
   guint keyval;
   GdkModifierType irrelevant;
   gdk_keymap_translate_keyboard_state(Client.keymap, event->hardware_keycode, 
         event->state, event->group, &keyval, NULL, NULL, &irrelevant);

   for(guint i=0; i<LENGTH(shortcuts); i++){
      regex_t regex;
      Shortcut* sc = (Shortcut*)&shortcuts[i];
      
      int     status=1;
      if(sc->regex==NULL) status = Client.buffer==NULL ? 0 : 1;
      else {
         regcomp(&regex, sc->regex, REG_EXTENDED);
         if(Client.buffer!=NULL)
            status = regexec(&regex, Client.buffer->str, (size_t) 0, NULL, 0);
         regfree(&regex);
      }

      if ( keyval == sc->key                                         // test key
            && (event->state & ~irrelevant & ALL_MASK) == sc->mask   // test mask
            && (status==0 || keyval == GDK_KEY_Escape)
            && Client.mode != INSERT
            && sc->function
         ) {
            sc->function(&(sc->argument));
            if(Client.buffer!=NULL){
               g_string_free(Client.buffer, TRUE);
               Client.buffer = NULL;
               gtk_label_set_text((GtkLabel*) Client.Statusbar.message, "");     
            }
         return TRUE;
      }
   }
   
   // stop here in case of PASS_THROUGH mode
   if(Client.mode == PASS_THROUGH) return FALSE;

   // append only numbers and characters to buffer
   if(Client.mode==NORMAL && isascii(keyval)) {
      if(!Client.buffer)
         Client.buffer = g_string_new("");

      Client.buffer = g_string_append_c(Client.buffer, keyval);
      gtk_label_set_text((GtkLabel*) Client.Statusbar.message, Client.buffer->str);
   }

   return TRUE;
}


//--- inputbar callbacks ---
gboolean cb_inputbar_keypress(GtkWidget* widget, GdkEventKey* event, gpointer data) {

   guint keyval;
   GdkModifierType irrelevant;
   gdk_keymap_translate_keyboard_state(Client.keymap, event->hardware_keycode, 
         event->state, event->group, &keyval, NULL, NULL, &irrelevant);

   gchar  *input  = gtk_editable_get_chars(GTK_EDITABLE(Client.UI.inputbar), 0, -1);
   if(keyval==GDK_KEY_Escape || (strlen(input)<=1 && keyval==GDK_KEY_BackSpace))
      clear_inputbar();
   
   //--- inputbar shortcuts -----
   switch (keyval) {
      case GDK_KEY_Tab:
         run_completion(NEXT_GROUP);   return TRUE;
      case GDK_KEY_Up:
         run_completion(PREVIOUS);     return TRUE;
      case GDK_KEY_Down:
         run_completion(NEXT);         return TRUE;
   }

   return FALSE;
}

gboolean cb_inputbar_activate(GtkEntry* entry, gpointer data) {
   gchar    *input      = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
   gchar     identifier = input[0];
   //gboolean  retv       = FALSE;
   gboolean  succ       = FALSE;

   say(DEBUG, "cb_inputbar_activate", -1);
   
   // clear message
   gtk_label_set_text((GtkLabel*) Client.Statusbar.message, "");
   
   // no input 
   if(strlen(input) <= 1) {
      clear_inputbar();
      g_free(input);
      return FALSE;
   }

   // search mode 
   if(identifier == '/'){
      search_and_highlight(FORWARD, (gchar*)input+1);

      clear_inputbar();
      g_free(input);
      return TRUE;
   }
   
   // command mode
   if(identifier == ':') {
      gchar **tokens = g_strsplit(input + 1, " ", -1);
      g_free(input);
      gchar *command = tokens[0];
      gint    length = g_strv_length(tokens);

      // search commands
      for(unsigned int i = 0; i < LENGTH(commands); i++) {
         Command* c = (Command*)&commands[i];
         if((g_strcmp0(command, c->command) == 0)) {
            c->function(length - 1, tokens + 1);
            succ = TRUE;
            break;
         }
      }
      g_strfreev(tokens);
   }

   if(!succ) notify(ERROR, "Unknown command");
   
   //clear_inputbar(); 
   run_completion(HIDE);
   set_inputbar_visibility(HIDE);
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_PAGE()));
   
   return TRUE;
}


//--- webview callbacks ---
gboolean cb_wv_load_status(WebKitWebView* wv, WebKitLoadEvent load_event, gpointer user_data){

      //WebKitLoadStatus status = webkit_web_view_get_load_status(wv);

      /*if(status==WEBKIT_LOAD_COMMITTED){
         gchar *buffer = Client.Global.user_script->content;
         if(!buffer){
            notify(WARNING, "No script loaded");
            return FALSE;
         }
         run_script(buffer, NULL, NULL);

         // Flashblock stuff
         if(fb_enabled){
            FBFrames = NULL;
            WebKitDOMDocument *doc = webkit_web_view_get_dom_document(wv);
            WebKitDOMDOMWindow *win = webkit_dom_document_get_default_view(doc);
            webkit_dom_event_target_add_event_listener(WEBKIT_DOM_EVENT_TARGET(win), "beforeload", G_CALLBACK(cb_flashblock_before_load), TRUE, NULL);
            webkit_dom_event_target_add_event_listener(WEBKIT_DOM_EVENT_TARGET(win), "beforeunload", G_CALLBACK(cb_flashblock_before_unload), TRUE, NULL);
         }

      } else */
      if(load_event==WEBKIT_LOAD_FINISHED){
      
         update_client(gtk_notebook_get_current_page(Client.UI.notebook));
         
         gchar* uri = (gchar*) webkit_web_view_get_uri(wv);
         if(!uri) return FALSE;

         //--- Update history -----
         if(!private_mode) {
            // we verify if the new_uri is already present in the list
            GList* l = g_list_find_custom(Client.history, uri, (GCompareFunc)strcmp);
            if (l) {
               // new_uri is already present -> move it to the end of the list
               Client.history = g_list_remove_link(Client.history, l);
               Client.history = g_list_concat(l, Client.history);
            } else 
               Client.history = g_list_prepend(Client.history, g_strdup(uri));
         }
      }

      return TRUE;
}


