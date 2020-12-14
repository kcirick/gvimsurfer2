#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <webkit2/webkit2.h>

#include "include/gvimsurfer2.h"
#include "include/client.h"

gboolean private_mode;

gint   default_width;
gint   default_height;
gint   max_title_length;
gint   max_url_length;

gboolean fb_enabled;
gboolean full_content_zoom;
gboolean show_tabbar;
gboolean strict_ssl; 

gchar* user_agent;
gchar* home_page;

gint n_completion_items;
gint history_limit;
gfloat zoom_step;
gfloat scroll_step;

gchar* download_dir;
gchar* bookmarks;
gchar* history;
gchar* cookies;
gchar* sessions;
gchar* stylesheet;
gchar* scriptfile;
gchar* ui_cssfile;

//--- Local variables -----
static const gchar default_config_file[] = "configrc";
gboolean mode_debug;

#define CRED     "\x1b[31m"
#define CGREEN   "\x1b[32m"
#define CYELLOW  "\x1b[33m"
#define CPURPLE  "\x1b[35m"
#define CRESET   "\x1b[0m"

//--- Helper functions -----
gchar* build_proper_path(gchar* path){
   
   gchar* proper_path;
   if(path[0]=='~')        proper_path = g_build_filename(g_get_home_dir(), path+1, NULL);
   else if(path[0]=='/')   proper_path = g_strdup(path);
   else {
      notify(WARNING, g_strdup_printf("Path %s is vague", path));
      proper_path = g_strdup(path);
   }

   return proper_path;
}

gchar* shorten_text(gchar* orig, gint max_length){

   gchar* new_text = g_strdup_printf("%s", orig);
   if(strlen(orig) > max_length){
      g_strlcpy(new_text, orig, max_length-3);
      g_strlcat(new_text, "...", max_length);
   } 

   return new_text;
}

gint get_int_from_buffer(gchar* buffer){

   gint digit_end = 0;
   while(g_ascii_isdigit(buffer[digit_end]))
      digit_end = digit_end + 1;

   gchar* number = g_strndup(buffer, digit_end);
   gint id = atoi(number);
   g_free(number);

   return id;
}


void say(gint level, gchar* message, gint exit_type) {
   if(level==DEBUG && !mode_debug) return;

   gchar* coloured_type;
        if(level==ERROR)   coloured_type=g_strdup(CRED "ERROR" CRESET);
   else if(level==WARNING) coloured_type=g_strdup(CYELLOW "WARNING" CRESET);
   else if(level==DEBUG)   coloured_type=g_strdup(CPURPLE "DEBUG" CRESET);
   else                    coloured_type=g_strdup(CGREEN "INFO" CRESET);

   g_printf("%s [%s]:\t%s\n", NAME, coloured_type, message);

   if(exit_type==EXIT_SUCCESS || exit_type==EXIT_FAILURE ) 
      exit(exit_type);
}

void notify(gint level, char* message) {

   if(level==DEBUG){
      say(level, message, -1);
      return;
   }

   (level==ERROR || level==WARNING) ?
      gtk_widget_set_state_flags (GTK_WIDGET(Client.Statusbar.message), GTK_STATE_FLAG_CHECKED, TRUE) :
      gtk_widget_set_state_flags (GTK_WIDGET(Client.Statusbar.message), GTK_STATE_FLAG_NORMAL, TRUE); 

   gtk_label_set_text((GtkLabel*) Client.Statusbar.message, message);
}

void open_uri(WebKitWebView* wv, const gchar* uri){
   if(!uri || strlen(uri)==0) return;

   gchar* new_uri = NULL;

   gchar **args = g_strsplit(uri, " ", -1);
   gint   nargs = g_strv_length(args);

   if(nargs==1){  // only one argument given

      // file path
      if(uri[0] == '/' || strncmp(uri, "./", 2) == 0) 
         new_uri = g_strconcat("file://", uri, NULL);
      // uri does contain any ".", ":" or "/" nor does it start with "localhost"
      else if(!strpbrk(uri, ".:/") && strncmp(uri, "localhost", 9)){
         SearchEngine* se = (SearchEngine*)Client.search_engines->data;
         new_uri = g_strdup_printf(se->uri, uri);
      } else
         new_uri = strstr(uri, "://") ? g_strdup(uri) : g_strconcat("http://", uri, NULL);
   
   } else {       // multiple arguments given -> search engine
      SearchEngine* se;
      gboolean matched=FALSE;
      for(GList* list = Client.search_engines; list; list = g_list_next(list)){
         se = (SearchEngine*)list->data;
         if(g_strcmp0(args[0], se->name)==0){ matched=TRUE; break; }
      }

      if(!matched){
         // If not matched, use the default search engine
         notify(WARNING, g_strdup_printf("Search engine %s doesn't exist", args[0]));
         se = (SearchEngine*)Client.search_engines->data;
         uri = g_strjoinv(" ", args);
      } else 
         uri = g_strjoinv(" ", &args[1]);

      new_uri = g_strdup_printf(se->uri, uri);
   }
   
   webkit_web_view_load_uri(wv, new_uri);

   g_strfreev(args);
   g_free(new_uri);
}

void new_window(const gchar* uri) {
   if(!uri) return;

   gchar* nargv[3] = { TARGET, (gchar*)uri, NULL };
   g_spawn_async(NULL, nargv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

void search_and_highlight(gboolean direction, gchar* token) {
   static WebKitWebView* last_wv = NULL;
   gboolean search_handle_changed = FALSE;


   if(!Client.search_token || !strlen(Client.search_token)) {
      if(token)   Client.search_token = g_strdup(token);
      else        return;
   }

   if(token && strcmp(Client.search_token, token)!=0){
      Client.search_token = g_strdup(token);
      search_handle_changed = TRUE;
   }

   WebKitWebView* current_wv = (WebKitWebView*)GET_CURRENT_PAGE();
   Client.finder = webkit_web_view_get_find_controller(current_wv);

   if(search_handle_changed || last_wv != current_wv) {
      webkit_find_controller_count_matches(Client.finder, Client.search_token,
         WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND,
         G_MAXUINT);

      last_wv = current_wv;
   }
   webkit_find_controller_search(Client.finder, Client.search_token,
      WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND |
      (direction ?  WEBKIT_FIND_OPTIONS_NONE : WEBKIT_FIND_OPTIONS_BACKWARDS),
      G_MAXUINT);
}

void change_mode(gint mode) {
   say(DEBUG, "change_mode", -1);
   /*
   gchar* mode_text = NULL;

   switch(mode) {
      case INSERT:
         mode_text = "-- INSERT --";
         break;
      case HINTS:
         mode_text = "-- HINTS -- ";
         break;
      case PASS_THROUGH:
         mode_text = "-- PASS THROUGH --";
         break;
      default:
         mode_text = "";
         mode = NORMAL;
         gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_PAGE()));
   }
   Client.mode = mode;
   gtk_label_set_text((GtkLabel*) Client.Statusbar.message, mode_text);
   */
}

gboolean sessionsave(gchar* session_name) {
   GString* session_uris = g_string_new("");

   for (int i = 0; i < gtk_notebook_get_n_pages(Client.UI.notebook); i++) {

      gchar* tab_uri   = g_strconcat(webkit_web_view_get_uri((WebKitWebView*)GET_NTH_PAGE(i)), " ", NULL);
      session_uris     = g_string_append(session_uris, tab_uri);

      g_free(tab_uri);
   }

   // Check for existing sessions
   GList* se_list = Client.sessions;
   while(se_list) {
      Session* se = se_list->data;

      if(g_strcmp0(se->name, session_name) == 0) {
         g_free(se->uris);
         se->uris = session_uris->str;

         break;
      }
      se_list = g_list_next(se_list);
   }

   if(!se_list) {
      Session* se = malloc(sizeof(Session));
      se->name = g_strdup(session_name);
      se->uris = session_uris->str;

      Client.sessions = g_list_prepend(Client.sessions, se);
   }

   g_string_free(session_uris, FALSE);

   return TRUE;

}

gboolean sessionload(gchar* session_name) {
   GList* se_list = Client.sessions;
   while(se_list) {
      Session* se = se_list->data;

      if(g_strcmp0(se->name, session_name) == 0) {
         gchar** uris = g_strsplit(se->uris, " ", -1);
         int     n    = g_strv_length(uris) - 1;

         if(n <= 0)  return FALSE;

         for(int i = 0; i < n; i++){
            say(DEBUG, uris[i], -1);
            create_tab(uris[i], TRUE);
         }

         g_strfreev(uris);
         say(DEBUG, "here", -1);
         return TRUE;
      }
      se_list = g_list_next(se_list);
   }

   return FALSE;
}


gboolean read_configuration(gchar* configrc) {
   if(!configrc) return FALSE;
   if(!g_file_test(configrc, G_FILE_TEST_IS_REGULAR)) return FALSE;

   gchar* content = NULL;
   if(!g_file_get_contents(configrc, &content, NULL, NULL)) return FALSE;

   gchar **lines = g_strsplit(content, "\n", -1);
   gint    n     = g_strv_length(lines) - 1;

   for(gint i = 0; i <= n; i++) {
      if(!strlen(lines[i]) || lines[i][0]=='#') continue;

      gchar* id = strtok(lines[i], "=");
      gchar* value = strtok(NULL, "\n");

      // remove whitespaces
      id = g_strstrip(id);
      value = g_strstrip(value);
      if(!strlen(value)) continue;

      if(!strcmp(id, "default_width"))    default_width = atoi(value);
      if(!strcmp(id, "default_height"))   default_height = atoi(value);
      if(!strcmp(id, "max_title_length")) max_title_length = atoi(value);
      if(!strcmp(id, "max_url_length"))   max_url_length = atoi(value);

      if(!strcmp(id, "full_content_zoom"))   full_content_zoom = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_tabbar"))         show_tabbar = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "strict_ssl"))          strict_ssl = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "flash_block"))         fb_enabled = strcmp(value, "false") ? TRUE : FALSE;

      if(!strcmp(id, "home_page"))        home_page = value;
      if(!strcmp(id, "user_agent"))       user_agent = value;

      if(!strcmp(id, "n_completion_items"))    n_completion_items = atoi(value);
      if(!strcmp(id, "history_limit"))         history_limit = atoi(value);
      if(!strcmp(id, "zoom_step"))             zoom_step = atof(value);
      if(!strcmp(id, "scroll_step"))           scroll_step = atof(value);

      if(!strcmp(id, "download_dir")) download_dir = build_proper_path(value);
      if(!strcmp(id, "bookmarks"))    bookmarks    = g_strconcat(CONFIGDIR, "/", value, NULL);
      if(!strcmp(id, "history"))      history      = g_strconcat(CONFIGDIR, "/", value, NULL);
      if(!strcmp(id, "cookies"))      cookies      = g_strconcat(CONFIGDIR, "/", value, NULL);
      if(!strcmp(id, "sessions"))     sessions     = g_strconcat(CONFIGDIR, "/", value, NULL);
      if(!strcmp(id, "stylesheet"))   stylesheet   = g_strconcat(CONFIGDIR, "/", value, NULL);
      if(!strcmp(id, "ui_css"))       ui_cssfile   = g_strconcat(CONFIGDIR, "/", value, NULL);
      //if(!strcmp(id, "scriptfile"))   load_script(g_strconcat(CONFIGDIR, value, NULL));

      // Search Engines
      if(!strcmp(id, "search_engine")){
         gchar **entries = g_strsplit_set(value, " ", -1);
         gint    num_entries = g_strv_length(entries);
         if(num_entries !=2) notify(WARNING, "Numbers of entries is not 2!");
         
         SearchEngine* sengine = malloc(sizeof(SearchEngine));
         sengine->name = entries[0];
         sengine->uri  = entries[1];

         Client.search_engines = g_list_append(Client.search_engines, sengine);
         g_free(entries);
      }
   }
   g_free(lines);

   return TRUE;
}


//------------------------------------------------------------------------
// Main function
//----------------
int main(int argc, char* argv[]) {

   static GError *err;
   static gboolean version = FALSE;
   static gboolean private = FALSE;
   static const gchar* cfile = NULL;
   static GOptionEntry opts[] = {
      { "version",      'v', 0, G_OPTION_ARG_NONE, &version, "Print version", NULL },
      { "configfile",   'c', 0, G_OPTION_ARG_STRING, &cfile, "Specify config file", NULL },
      { "private",      'p', 0, G_OPTION_ARG_NONE, &private, "Open in private mode", NULL },
      { "debug",        'd', 0, G_OPTION_ARG_NONE, &mode_debug, "Set debug mode", NULL },
      { NULL } };

   if (!gtk_init_with_args(&argc, &argv, "[URL1 URL2 ...]", opts, NULL, &err))
      say(ERROR, err->message, EXIT_FAILURE);

   if (version)
      say(INFO, "Version "VERSION, EXIT_SUCCESS);

   //--- read config file -----
   gchar* configfile= cfile ?
      g_strdup(cfile): g_build_filename(CONFIGDIR, default_config_file, NULL);

   if(!read_configuration(configfile)) 
      say(ERROR, g_strdup_printf("Invalid configuration file: %s", configfile), EXIT_FAILURE);

   //--- init client -----
   init_client();

   // put private browsing flag after initialization
   private_mode = private;

   //--- create tab -----
   if(argc < 2)
      create_tab(NULL, TRUE);
   else
      for(gint i=1; i<argc; i++)
         create_tab(argv[i], TRUE);

   gtk_widget_show_all(GTK_WIDGET(Client.UI.window));
   gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));

   gtk_main();

   return EXIT_SUCCESS;
}

