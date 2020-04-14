#ifndef VIMSURFER_H
#define VIMSURFER_H

//--- macros --------
#define LENGTH(x) sizeof(x)/sizeof((x)[0])

#define GET_CURRENT_PAGE() GET_NTH_PAGE(gtk_notebook_get_current_page(Client.UI.notebook))
#define GET_NTH_PAGE(n)    gtk_notebook_get_nth_page(Client.UI.notebook, n)

//--- enums ---------
enum MesageType { DEBUG, INFO, WARNING, ERROR, NMSG };
enum Mode { NORMAL, INSERT, HINTS, PASS_THROUGH, NMODES };
enum { SHOW, HIDE, TOGGLE, HIGHLIGHT,
       BACKWARD, FORWARD,
       PREVIOUS_GROUP, PREVIOUS, NEXT, NEXT_GROUP 
};

//--- typedefs ------
typedef struct {
   gboolean b;
   gint     i;
   void*    data;
} Argument;

typedef struct {
   gchar*   name;
   gchar*   uri;
} SearchEngine;

typedef struct {
   gchar* uri;
   gchar* tags;
} BMark;

//--- variables -----
gboolean private_mode;

//--- configuration variables -----
gint   default_width;
gint   default_height;
gint   max_title_length;
gint   max_url_length;

gboolean fb_enabled;
gboolean full_content_zoom;
gboolean show_statusbar;
gboolean show_tabbar;
gboolean strict_ssl; 

gchar* user_agent;
gchar* home_page;
gchar* external_editor;

gint n_completion_items;
gint history_limit;
gfloat zoom_step;
gfloat scroll_step;

gchar* download_dir;
gchar* config_dir;
gchar* bookmarks;
gchar* history;
gchar* cookies;
gchar* sessions;
gchar* stylesheet;
gchar* scriptfile;
gchar* ui_cssfile;

//--- functions -----
void say(gint, gchar*, gint);
void notify(gint, gchar*);
void open_uri(WebKitWebView*, const gchar*);
void change_mode(gint mode);
void search_and_highlight(gboolean, gchar*);
void new_window(const gchar*);
gchar* shorten_text(gchar*, gint);

#endif
