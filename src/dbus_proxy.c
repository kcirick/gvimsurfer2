#include <gio/gio.h>
#include <glib.h>
#include <webkit2/webkit2.h>

#include "include/dbus_proxy.h"
#include "include/gvimsurfer2.h"
#include "include/client.h"

#define DBUS_OBJ_PATH  "/org/gvimsurfer2/browser/WebExtension"
#define DBUS_INTERFACE "org.gvimsurfer2.browser.WebExtension"

static GDBusServer *dbusserver;
static GDBusProxy* dbusproxy;

static gboolean cb_dbus_vscroll(GDBusConnection *connection,
   const char* sender_name, const char* object_path, const char* interface_name, const char* signal_name,
   GVariant *parameters, gpointer data) {
   
   say(DEBUG, "cb_dbus_vscoll", -1);
   
   glong max, top;
   guint percent, pageid;
   g_variant_get(parameters, "(ttqt)", &pageid, &max, &percent, &top);
   guint nb_pageid = gtk_notebook_get_current_page(Client.UI.notebook);
   say(DEBUG, g_strdup_printf("page id = %u(%u), max = %li, percent = %u, top = %li", pageid, nb_pageid, max, percent, top), -1);

   Client.ScrollState.max = max;
   Client.ScrollState.top = top;
   Client.ScrollState.percent = percent;

   update_statusbar_info( nb_pageid );

   return TRUE;
}

static gboolean cb_dbus_page_created(GDBusConnection *connection, 
   const char* sender_name, const char* object_path, const char* interface_name, const char* signal_name,
   GVariant *parameters, gpointer data) {
   
   say(DEBUG, "cb_dbus_page_created", -1);

   guint64 pageid;
   g_variant_get(parameters, "(t)", &pageid);
   say(DEBUG, g_strdup_printf("page id is %lu", pageid), -1);

   dbusproxy = (GDBusProxy*)data;

   g_dbus_connection_signal_subscribe(connection, NULL,
         DBUS_INTERFACE, "VerticalScroll",
         DBUS_OBJ_PATH, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
         (GDBusSignalCallback)cb_dbus_vscroll, NULL,
         NULL);
   
   return TRUE;
}

static gboolean cb_dbus_proxy_created(GDBusProxy* proxy, GAsyncResult *result, gpointer data) {

   say(DEBUG, "cb_dbus_proxy_created", -1);

   GError * error = NULL;
   GDBusProxy *finish_proxy = g_dbus_proxy_new_finish(result, &error);
   if(!finish_proxy) {
      say(ERROR, "Error creating web extension proxy", -1);
      g_error_free(error);

      return FALSE;
   }

   GDBusConnection *connection = g_dbus_proxy_get_connection(finish_proxy);
   g_dbus_connection_signal_subscribe(connection, NULL,
         DBUS_INTERFACE, "PageCreated",
         DBUS_OBJ_PATH, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
         (GDBusSignalCallback)cb_dbus_page_created, proxy, 
         NULL);

   return TRUE;
}

static gboolean cb_dbus_close_connection(GDBusConnection* connection, gboolean remote_peer_vanished, GError *error, gpointer data) {
   if (error && !remote_peer_vanished){
      say(ERROR, "Unexpected lost connection to web extension", -1);
      return FALSE;
   }
   return TRUE;
}

static gboolean cb_dbus_new_connection(GDBusServer *server, GDBusConnection *connection, gpointer data){
   say(DEBUG, "cb_dbus_new_connection", -1);

   // Create dbus proxy
   g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);

   g_signal_connect(connection, "closed", G_CALLBACK(cb_dbus_close_connection), NULL);

   g_dbus_proxy_new(connection,
      G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES|G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
      NULL,
      NULL,
      DBUS_OBJ_PATH,
      DBUS_INTERFACE,
      NULL,
      (GAsyncReadyCallback)cb_dbus_proxy_created,
      NULL);

   return TRUE;
}

static gboolean cb_dbus_authorize(GDBusAuthObserver *observer, GIOStream *stream, GCredentials *credentials, gpointer data){
   say(DEBUG, "cb_dbus_authorize", -1);

   gboolean authorized = FALSE;

   if(credentials){
      GError* error = NULL;
      GCredentials* new_credentials = g_credentials_new();
      if (g_credentials_is_same_user(credentials, new_credentials, &error)) {
         authorized = TRUE;
      } else {
         say(ERROR, "Failed to authorize web extension connection", -1);
         g_error_free(error);
      }
      g_object_unref(new_credentials);
   } else {
      say(ERROR, "No credentials received from web extension", -1);
   }
   
   return authorized;
}

const gchar* dbus_proxy_init() {

   say(DEBUG, "dbus_proxy_init", -1);

   gchar* address = g_strdup_printf("unix:tmpdir=%s", g_get_tmp_dir());
   gchar* guid    = g_dbus_generate_guid();
   GDBusAuthObserver* observer = g_dbus_auth_observer_new();
   GError *error = NULL;

   g_signal_connect(observer, "authorize-authenticated-peer", G_CALLBACK(cb_dbus_authorize), NULL);

   dbusserver = g_dbus_server_new_sync(address, G_DBUS_SERVER_FLAGS_NONE, guid, observer, NULL, &error);

   if(error) {
      say(ERROR, "Failed to start web extension server", -1);
      g_error_free(error);
      g_free(address);
      g_free(guid);
      g_object_unref(observer);
      
      return NULL;
   }

   g_signal_connect(dbusserver, "new_connection", G_CALLBACK(cb_dbus_new_connection), NULL);
   g_dbus_server_start(dbusserver);

   return g_dbus_server_get_client_address(dbusserver);
}


void cb_dbus_user_scroll(Argument *arg){

   guint pageid = 0;
   gchar* direction = arg->b ? "up" : "down";

   //dbus_call("VerticalScroll", g_variant_new("(ts)", pageid, direction), NULL);
   g_dbus_proxy_call(dbusproxy, 
      "UserScroll", 
      g_variant_new("(ts)", pageid, direction),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
}
