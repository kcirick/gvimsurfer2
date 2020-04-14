#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "include/gvimsurfer2.h"
#include "include/shortcuts.h"
#include "include/client.h"
#include "include/completion.h"
#include "include/commands.h"

void sc_abort(Argument* argument){
   say(DEBUG, "sc_abort", -1);
   // Clear hints
   //gchar* cmd = "clear()";
   //run_script(cmd, NULL, NULL);

   // Stop loading website
   cmd_stop(0, NULL);

   // Set back to normal mode
   change_mode(NORMAL);

   // Hide inputbar
   clear_inputbar();

   // Unmark search results
   //webkit_web_view_unmark_text_matches(GET_CURRENT_TAB());

   // reset the statubar message fg colour 
   //gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.message), GTK_STATE_NORMAL, &(Client.Style.statusbar_fg));
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_PAGE()));

}

void sc_focus_inputbar(Argument* argument) {
   say(DEBUG, "sc_focus_inputbar", -1);

   if(argument->data) {
      gchar* data = argument->data;
      //if(argument->b)
      //   data = g_strdup_printf("%s%s", data, webkit_web_view_get_uri(GET_CURRENT_TAB()));
      //else
         data = g_strdup(data);

      gtk_entry_set_text(Client.UI.inputbar, data);
      g_free(data);

      /* we save the X clipboard that will be clear by "grab_focus" */
      //gchar* x_clipboard_text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

      gtk_widget_grab_focus(GTK_WIDGET(Client.UI.inputbar));
      gtk_editable_set_position(GTK_EDITABLE(Client.UI.inputbar), -1);

      //if (x_clipboard_text != NULL) {
      //   /* we reset the X clipboard with saved text */
      //   gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), x_clipboard_text, -1);

      //   g_free(x_clipboard_text);
      //}
   }

   set_inputbar_visibility(TOGGLE);
}

void sc_search(Argument* argument) {
   search_and_highlight(argument->b, argument->data);
}
