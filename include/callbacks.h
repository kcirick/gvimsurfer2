#ifndef CALLBACKS_H
#define CALLBACKS_H

gboolean    cb_blank();
gboolean    cb_destroy(GtkWidget*, gpointer);

gboolean    cb_notebook_switch_page(GtkNotebook*, gpointer, guint, gpointer);
gboolean    cb_notebook_keypress(GtkWidget*, GdkEventKey*, gpointer);

gboolean    cb_button_close_tab(GtkButton*, GtkNotebook*);
//gboolean    cb_button_close_tab(GtkWidget*, GdkEventButton*, gpointer);


gboolean    cb_inputbar_keypress(GtkWidget*, GdkEventKey*, gpointer);
gboolean    cb_inputbar_activate(GtkEntry*, gpointer);

gboolean    cb_wv_load_status(WebKitWebView*, WebKitLoadEvent, gpointer);

#endif
