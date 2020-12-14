#ifndef VIMSURFER_H
#define VIMSURFER_H

//--- macros --------
#define LENGTH(x) sizeof(x)/sizeof((x)[0])

#define GET_CURRENT_PAGE() GET_NTH_PAGE(gtk_notebook_get_current_page(Client.UI.notebook))
#define GET_NTH_PAGE(n)    gtk_notebook_get_nth_page(Client.UI.notebook, n)

#define CONFIGDIR g_build_path("/", g_get_home_dir(),".config/gvimsurfer2", NULL)

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

typedef struct {
   gint     id;
   gchar*   uri;
} QMark;

typedef struct {
   gchar*   name;
   gchar*   uris;
} Session;

//--- variables -----
extern gboolean private_mode;

//--- configuration variables -----
extern gint   default_width;
extern gint   default_height;
extern gint   max_title_length;
extern gint   max_url_length;

extern gboolean fb_enabled;
extern gboolean full_content_zoom;
extern gboolean show_tabbar;
extern gboolean strict_ssl; 

extern gchar* user_agent;
extern gchar* home_page;

extern gint n_completion_items;
extern gint history_limit;
extern gfloat zoom_step;
extern gfloat scroll_step;

extern gchar* download_dir;
extern gchar* bookmarks;
extern gchar* history;
extern gchar* cookies;
extern gchar* sessions;
extern gchar* stylesheet;
extern gchar* scriptfile;
extern gchar* ui_cssfile;

//--- functions -----
void     say(gint, gchar*, gint);
void     notify(gint, gchar*);
void     open_uri(WebKitWebView*, const gchar*);
void     change_mode(gint mode);
void     search_and_highlight(gboolean, gchar*);
void     new_window(const gchar*);
gchar*   shorten_text(gchar*, gint);
gint     get_int_from_buffer(gchar*);
gboolean sessionsave(gchar*);
gboolean sessionload(gchar*);

#endif
