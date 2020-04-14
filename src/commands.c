#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "include/gvimsurfer2.h"
#include "include/client.h"
#include "include/completion.h"
#include "include/commands.h"

gboolean cmd_back(int argc, char** argv) {
   //gint increment = argc==0 ? 1 : atoi(argv[0]);
   webkit_web_view_go_back((WebKitWebView*)GET_CURRENT_PAGE());

   return TRUE;
}

gboolean cmd_forward(int argc, char** argv) {
   //gint increment = argc==0 ? 1 : atoi(argv[0]);
   webkit_web_view_go_forward((WebKitWebView*)GET_CURRENT_PAGE());

   return TRUE;
}

gboolean cmd_open(int argc, char** argv) {
  if(argc < 1)             return TRUE;
  if(argv[argc] != NULL)   return TRUE;

  gchar* uri = g_strjoinv(" ", argv);
  if(strlen(uri)>0)
     open_uri((WebKitWebView*)GET_CURRENT_PAGE(), uri);

  g_free(uri);

  return TRUE;
}

gboolean cmd_close(int argc, char** argv) {
   close_tab(gtk_notebook_get_current_page(Client.UI.notebook));
   return TRUE;
}

gboolean cmd_closeall(int argc, char** argv) {

   //pango_font_description_free(Client.Style.font);

   // write bookmarks and history
   if(!private_mode)   cmd_write(0, NULL);

   // clear bookmarks
   for(GList* list = Client.bookmarks; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.bookmarks);

   // clear history
   for(GList* list = Client.history; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.history);

   // clean search engines 
   for(GList* list = Client.search_engines; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.search_engines);

   // clean quickmarks
   //for(GList* list = Client.Global.quickmarks; list; list = g_list_next(list))
   //   free(list->data);

   //g_list_free(Client.Global.quickmarks);
   
   // quit application
   gtk_main_quit();

   return TRUE;
}

gboolean cmd_reload(int argc, char** argv) {
   if(argc==0){
      webkit_web_view_reload((WebKitWebView*)GET_CURRENT_PAGE());
   } else if (strcmp(argv[0], "all")==0){
      int number_of_tabs = gtk_notebook_get_n_pages(Client.UI.notebook);

      for(int i=0; i<number_of_tabs; i++)
         webkit_web_view_reload((WebKitWebView*)GET_NTH_PAGE(i));
   }
   return TRUE;
}

gboolean cmd_stop(int argc, char** argv) {
   if(argc==0){
      webkit_web_view_stop_loading((WebKitWebView*)GET_CURRENT_PAGE());
   } else if (strcmp(argv[0], "all")==0){
      int number_of_tabs = gtk_notebook_get_n_pages(Client.UI.notebook);

      for(int i=0; i<number_of_tabs; i++)
         webkit_web_view_stop_loading((WebKitWebView*)GET_NTH_PAGE(i));
   }
   return TRUE;
}


gboolean cmd_tabopen(int argc, char** argv) {
   if(argc == 0)           return TRUE;
   //if(argv[argc] != NULL)  return TRUE;

   gchar* uri = g_strjoinv(" ", argv);
   if(strlen(uri)>0)
      create_tab(uri, FALSE);

   g_free(uri);

   return TRUE;
}

gboolean cmd_winopen(int argc, char** argv) {
   if(argc == 0)   return TRUE;

   gchar* uri = g_strjoinv(" ", argv);
   if(strlen(uri)>0)
      new_window(uri);

   g_free(uri);

   return TRUE;
}

gboolean cmd_bookmark(int argc, char** argv) {
   gchar* this_uri = g_strdup(webkit_web_view_get_uri((WebKitWebView*)GET_CURRENT_PAGE()));

   // Verify that the bookmark isn't already in the list
   for(GList* l = Client.bookmarks; l; l = g_list_next(l)) {
      BMark *this_bmark = (BMark*) l->data;
      if(!g_strcmp0(this_uri, this_bmark->uri)) return TRUE;
   }

   BMark* new_bmark = malloc(sizeof(BMark));
   gchar* tags = "";
   if(argc >= 1 && argv[argc] == NULL)
      tags = g_strjoinv(" ", argv);
   new_bmark -> uri = g_strdup(this_uri);
   new_bmark -> tags = g_strdup(tags);
   
   Client.bookmarks = g_list_append(Client.bookmarks, new_bmark);
   if(strlen(this_uri)>0)   g_free(this_uri); 
   if(strlen(tags)>0)       g_free(tags);

   return TRUE;
}

gboolean cmd_write(int argc, char** argv) {
   // save bookmarks
   GString *bookmark_list = g_string_new("");
   for(GList* l = Client.bookmarks; l; l = g_list_next(l)){
      BMark* bmark = (BMark*)l->data;
      gchar* bookmark = g_strconcat(bmark->uri, " ", bmark->tags ? bmark->tags : "", "\n", NULL);
      bookmark_list = g_string_append(bookmark_list, bookmark);
      g_free(bookmark);
   }

   g_file_set_contents(bookmarks, bookmark_list->str, -1, NULL);

   g_string_free(bookmark_list, TRUE);

   // save history 
   int h_counter = 0;

   GString *history_list = g_string_new("");
   for(GList* h=Client.history; h && (!history_limit || h_counter<history_limit); h=g_list_next(h)) {
      gchar* uri = g_strconcat((gchar*) h->data, "\n", NULL);
      history_list = g_string_append(history_list, uri);
      g_free(uri);

      h_counter++;
   }

   g_file_set_contents(history, history_list->str, -1, NULL);

   g_string_free(history_list, TRUE);

   return TRUE;
}


