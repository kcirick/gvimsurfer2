#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "include/gvimsurfer2.h"
#include "include/client.h"
#include "include/completion.h"
#include "include/commands.h"
#include "include/shortcuts.h"

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

   say(DEBUG, "cmd_closeall", -1);

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
   for(GList* list = Client.quickmarks; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.quickmarks);
   
   // clean downloads 
   for(GList* list = Client.active_downloads; list; list = g_list_next(list))
      free(list->data);
   for(GList* list = Client.finished_downloads; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.active_downloads);
   g_list_free(Client.finished_downloads);
   
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
   gchar* uri = (argc==0 || strlen(argv[0])==0) ? g_strdup(home_page) : g_strjoinv(" ", argv);

   if(strlen(uri)>0)
      create_tab(uri, FALSE);

   g_free(uri);

   return TRUE;
}

gboolean cmd_winopen(int argc, char** argv) {
   gchar* uri = (argc==0 || strlen(argv[0])==0) ? g_strdup(home_page) : g_strjoinv(" ", argv);

   if(strlen(uri)>0)
      new_window(uri);

   g_free(uri);

   return TRUE;
}

gboolean cmd_bookmark(int argc, char** argv) {
   if(argc<1) return FALSE;

   if(!g_strcmp0(argv[0], "set")){
      
      gchar* this_uri = g_strdup(webkit_web_view_get_uri((WebKitWebView*)GET_CURRENT_PAGE()));

      // Verify that the bookmark isn't already in the list
      for(GList* l = Client.bookmarks; l; l = g_list_next(l)) {
         BMark *this_bmark = (BMark*) l->data;
         if(!g_strcmp0(this_uri, this_bmark->uri)) return TRUE;
      }

      BMark* new_bmark = malloc(sizeof(BMark));
      gchar* tags = "";
      if(argc > 1 && argv[argc] == NULL)
         tags = g_strjoinv(" ", ++argv);
      new_bmark -> uri = g_strdup(this_uri);
      new_bmark -> tags = g_strdup(tags);
   
      Client.bookmarks = g_list_append(Client.bookmarks, new_bmark);
      if(strlen(this_uri)>0)   g_free(this_uri); 
      if(strlen(tags)>0)       g_free(tags);
      
      return TRUE;
   } else
   if (!g_strcmp0(argv[0], "open")) {
      if (argc>1)
         cmd_open(--argc, ++argv);
      else
         run_completion(NEXT);
      
      return TRUE;
   }

   return FALSE;
}

gboolean cmd_quickmark(int argc, char** argv){
   if(argc<1) return FALSE;

   gint id = atoi(argv[1]);

   if(!g_strcmp0(argv[0], "set")){
      gchar* this_uri = g_strdup(webkit_web_view_get_uri((WebKitWebView*)GET_CURRENT_PAGE()));
      
      // search if entry already exists
      for(GList* list = Client.quickmarks; list; list = g_list_next(list)){
         QMark* qmark = (QMark*) list->data;   

         if(qmark->id == id){
            qmark->uri = this_uri;
            return TRUE;
         }
      }

      // add new qmark
      QMark* qmark = malloc(sizeof(QMark));
      qmark-> id   = id;
      qmark-> uri  = g_strdup(this_uri);

      Client.quickmarks = g_list_append(Client.quickmarks, qmark);
      g_free(this_uri);
   
      return TRUE;
   } else
   if(!g_strcmp0(argv[0], "open")) {
      if (argc>1){
         Argument arg = {.i = id};
         sc_quickmark(&arg);
      } else
         run_completion(NEXT);

      return TRUE;
   }

   return FALSE;
}

gboolean cmd_handle_downloads(int argc, char** argv){
   if(argc<1) return FALSE;

   if(!g_strcmp0(argv[0], "list")){
      run_completion(NEXT);
      return TRUE;
   }

   gint which = atoi(argv[argc-1]);
   WebKitDownload* this_download = (WebKitDownload*)g_list_nth_data(Client.active_downloads, which);
   if(!this_download){
      notify(WARNING, "No such download!");
      return FALSE;
   }

   if(!g_strcmp0(argv[0], "cancel")){
      webkit_download_cancel(this_download);

      notify(INFO, "Download cancelled");
   }

   return TRUE;
}

gboolean cmd_session(int argc, char** argv) {
   if(argc<=1 || argv[argc]!=NULL)  return FALSE;

   gchar* session_name   = argv[1];
   
   gboolean to_return=FALSE;
   if(strcmp(argv[0], "save")==0)
      to_return = sessionsave(session_name);
   else if(strcmp(argv[0], "load")==0)
      to_return = sessionload(session_name);

   g_free(session_name);

   return to_return;
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

   // save session
   sessionsave("last_session");

   GString* session_list = g_string_new("");
   for(GList* se_list = Client.sessions; se_list; se_list = g_list_next(se_list)) {
      Session* se = se_list->data;

      gchar* session_lines = g_strconcat(se->name, "\n", se->uris, "\n", NULL);
      session_list = g_string_append(session_list, session_lines);

      g_free(session_lines);
   }

   g_file_set_contents(sessions, session_list->str, -1, NULL);

   g_string_free(session_list, TRUE);


   return TRUE;
}


