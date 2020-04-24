#include <glib.h>
#include <webkit2/webkit-web-extension.h>
#include <webkitdom/webkitdom.h>

#define DBUS_OBJ_PATH  "/org/gvimsurfer2/browser/WebExtension"
#define DBUS_INTERFACE "org.gvimsurfer2.browser.WebExtension"

static GDBusConnection* connection;
static gint current_pageid;

static const char introspection_xml[] =
    "<node>"
    " <interface name='" DBUS_INTERFACE "'>"
//    "  <method name='EvalJs'>"
//    "   <arg type='t' name='page_id' direction='in'/>"
//    "   <arg type='s' name='js' direction='in'/>"
//    "   <arg type='b' name='success' direction='out'/>"
//    "   <arg type='s' name='result' direction='out'/>"
//    "  </method>"
//    "  <method name='EvalJsNoResult'>"
//    "   <arg type='t' name='page_id' direction='in'/>"
//    "   <arg type='s' name='js' direction='in'/>"
//    "  </method>"
//    "  <method name='FocusInput'>"
//    "   <arg type='t' name='page_id' direction='in'/>"
//    "  </method>"
    "  <signal name='PageCreated'>"
    "   <arg type='t' name='page_id' direction='out'/>"
    "  </signal>"
    "  <method name='UserScroll'>"
    "   <arg type='t' name='page_id' direction='in'/>"
    "   <arg type='s' name='up_down' direction='in'/>"
    "  </method>"
    "  <signal name='VerticalScroll'>"
    "   <arg type='t' name='page_id' direction='out'/>"
    "   <arg type='t' name='max' direction='out'/>"
    "   <arg type='q' name='percent' direction='out'/>"
    "   <arg type='t' name='top' direction='out'/>"
    "  </signal>"
//    "  <method name='SetHeaderSetting'>"
//    "   <arg type='s' name='headers' direction='in'/>"
//    "  </method>"
//    "  <method name='LockInput'>"
//    "   <arg type='t' name='page_id' direction='in'/>"
//    "   <arg type='s' name='elemend_id' direction='in'/>"
//    "  </method>"
//    "  <method name='UnlockInput'>"
//    "   <arg type='t' name='page_id' direction='in'/>"
//    "   <arg type='s' name='elemend_id' direction='in'/>"
//    "  </method>"
    " </interface>"
    "</node>";

static void user_scroll(WebKitDOMDocument* doc, char* direction){
   g_print("user_scroll\n");
   return;
}

static void dbus_handle_method_call(GDBusConnection *connection, 
   const char *sender, const char* object_path, const char* interface_name, const char* method,
   GVariant *params, GDBusMethodInvocation *invocation, gpointer extension) {

   char* value;
   guint64 pageid;

   if(!g_strcmp0(method, "UserScroll")) {
      g_variant_get(params, "(ts)", &pageid, &value);
      WebKitWebPage *page = webkit_web_extension_get_page(WEBKIT_WEB_EXTENSION(extension), pageid);
      if (!page) {
         g_warning("invalid page id %lu", pageid);
         g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR,
            G_DBUS_ERROR_INVALID_ARGS, "Invalid page id: %"G_GUINT64_FORMAT, pageid);
      }

      user_scroll(webkit_web_page_get_dom_document(page), value);
   }

   return;
}

static const GDBusInterfaceVTable interface_vtable = { dbus_handle_method_call, NULL, NULL};


static void dbus_emit_signal(const char* name, GVariant *data){

   GError *error = NULL;
   g_dbus_connection_emit_signal(connection, NULL,
      DBUS_OBJ_PATH, DBUS_INTERFACE,
      name, data, &error);

   if(error){
      g_warning("Failed to emit signal '%s': %s", name, error->message);
      g_error_free(error);
   }
}

static void cb_document_scroll(WebKitDOMEventTarget* target, WebKitDOMEvent* event, WebKitWebPage* page){
   g_print("cb_document_scroll\n");
   
   WebKitDOMDocument* doc;
   if(WEBKIT_DOM_IS_DOM_WINDOW(target))
      g_object_get(WEBKIT_DOM_DOM_WINDOW(target), "document", &doc, NULL);
   else
      doc = WEBKIT_DOM_DOCUMENT(target);
   if(!doc) return;

   WebKitDOMElement* de = webkit_dom_document_get_document_element(doc);
   if(!de) return;

   WebKitDOMElement* body = WEBKIT_DOM_ELEMENT(webkit_dom_document_get_body(doc));
   if(!body) return;

   glong max = 0, top =0, scrollTop, scrollHeight, clientHeight;
   guint percent = 0;

   scrollTop = MAX(webkit_dom_element_get_scroll_top(de),
            webkit_dom_element_get_scroll_top(body));

   clientHeight = webkit_dom_dom_window_get_inner_height( webkit_dom_document_get_default_view(doc) );

   scrollHeight = MAX(webkit_dom_element_get_scroll_height(de),
            webkit_dom_element_get_scroll_height(body));

   max = scrollHeight - clientHeight;
   if (max>0) {
      percent = (guint)(0.5 + (scrollTop * 100 / max));
      top = scrollTop;
   }

   dbus_emit_signal("VerticalScroll", 
      g_variant_new("(ttqt)", webkit_web_page_get_id(page), max, percent, top));
}

static void cb_webpage_send_request(WebKitWebPage* wp, WebKitURIRequest* req, WebKitURIResponse* res, gpointer ext) {
   g_print("cb_webpage_send_request\n");
}

static void cb_webpage_document_loaded(WebKitWebPage* wp, gpointer ext) {
   g_print("cb_webpage_document_loaded\n");
   
   WebKitDOMDocument *doc = webkit_web_page_get_dom_document(wp);
   WebKitDOMEventTarget *target = WEBKIT_DOM_EVENT_TARGET(webkit_dom_document_get_default_view(doc));

   // Observe scroll events to get current position in the document
   webkit_dom_event_target_add_event_listener(target, "scroll", G_CALLBACK(cb_document_scroll), FALSE, wp);

   // Call the callback explicitly 
   cb_document_scroll(target, NULL, wp);
}

static void cb_webpage_created(WebKitWebExtension *we, WebKitWebPage *wp, gpointer user_data) {
   current_pageid = webkit_web_page_get_id(wp);
   g_print("Page %d created for %s\n",  current_pageid, webkit_web_page_get_uri (wp));
   
   if(connection)
      dbus_emit_signal("PageCreated", g_variant_new("(t)", current_pageid));

   g_object_connect(wp,
         //"signal::send-request", G_CALLBACK(cb_webpage_send_request), we,
         "signal::document-loaded", G_CALLBACK(cb_webpage_document_loaded), we,
         NULL);
}

static gboolean cb_dbus_authorize(GDBusAuthObserver *observer, GIOStream *stream, GCredentials* credentials, gpointer extension){
   gboolean authorized = FALSE;
   if(credentials) {
      GError* error = NULL;
      GCredentials* new_credentials = g_credentials_new(); 
      if(g_credentials_is_same_user(credentials, new_credentials, &error)) {
         authorized = TRUE;
      } else {
         g_warning("Failed to authorize web extention connection: %s", error->message);
         g_error_free(error);
      }
      g_object_unref(new_credentials);
   } else 
      g_warning("No credentials received from UI process.\n");

   return authorized;
}


static gboolean cb_dbus_connection_created(GObject *source_object, GAsyncResult *result, gpointer data) {
   g_print("cb_dbus_connection_created\n");

   static GDBusNodeInfo *node_info = NULL;

   if(!node_info) node_info = g_dbus_node_info_new_for_xml(introspection_xml, NULL);

   GError *error = NULL;
   connection = g_dbus_connection_new_for_address_finish(result, &error);
   if (error) {
      g_warning("Failed to connect to UI process: %s", error->message);
      g_error_free(error);
      return FALSE;
   }

   // register the webextension object
   g_dbus_connection_register_object(connection,
      DBUS_OBJ_PATH,
      node_info->interfaces[0],
      &interface_vtable,
      WEBKIT_WEB_EXTENSION(data),
      NULL, &error);

   dbus_emit_signal("PageCreated", g_variant_new("(t)", current_pageid));
   return TRUE;
}

G_MODULE_EXPORT 
void webkit_web_extension_initialize_with_user_data (WebKitWebExtension *extension, GVariant *data) {
   
   gchar* server_address;
   g_variant_get(data, "(m&s)", &server_address);
   if(!server_address){
      g_warning("UI process did not start D-Bus server");
      return;
   }

   g_signal_connect (extension, "page-created", G_CALLBACK (cb_webpage_created), NULL);

   GDBusAuthObserver *observer = g_dbus_auth_observer_new();
   g_signal_connect(observer, "authorize-authenticated-peer", G_CALLBACK(cb_dbus_authorize), extension);
   
   g_dbus_connection_new_for_address(server_address,
      G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, observer, NULL,
      (GAsyncReadyCallback)cb_dbus_connection_created, extension);
   g_object_unref(observer);

}


