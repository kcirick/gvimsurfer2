#ifndef CLIENT_H
#define CLIENT_H

//--- the main client -----
struct {
   struct {
      GtkWidget      *window;
      GtkBox         *box;
      GtkNotebook    *notebook; 
      GtkBox         *statusbar;
      GtkEntry       *inputbar;
      GtkCssProvider *style_provider;
   } UI;

   struct {
      GtkLabel *message;
      GtkLabel *uri;
      GtkLabel *info;
   } Statusbar;

   GdkKeymap            *keymap;
   GString              *buffer;
   gint                 mode;
   GList                *search_engines;
   WebKitFindController *finder;
   gchar                *search_token;
   GList                *bookmarks;
   GList                *quickmarks;
   GList                *history;

} Client;

//--- functions -----
void        init_client();

void        close_tab(gint);
GtkWidget*  create_tab(const gchar*, gboolean);
void        update_client(gint);
void        update_statusbar_info(gint);
void        clear_inputbar();
void        set_inputbar_visibility(gint);


#endif
