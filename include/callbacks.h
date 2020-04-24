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

GtkWidget*  cb_wv_create_webview(WebKitWebView*, gpointer);
gboolean    cb_wv_update(WebKitWebView* wv, gpointer data);
gboolean    cb_wv_load_status(WebKitWebView*, WebKitLoadEvent, gpointer);
gboolean    cb_wv_notify_title(WebKitWebView*, GParamSpec*, gpointer);
//gboolean    cb_wv_scrolled(GtkAdjustment*, gpointer);

gboolean    cb_wc_download_started(WebKitWebContext*, WebKitDownload*, gpointer);
gboolean    cb_download_decide_destination(WebKitDownload*, gchar*, gpointer);
gboolean    cb_download_failed(WebKitDownload*, GError*, gpointer);
gboolean    cb_download_finished(WebKitDownload*, gpointer);

gboolean    cb_download_progress(WebKitDownload*, GParamSpec*);

gboolean    cb_wc_init_webext(WebKitWebContext*, gpointer);

#endif
