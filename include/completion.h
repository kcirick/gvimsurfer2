#ifndef COMPLETION_H
#define COMPLETION_H

typedef struct {
   gchar *command;
   gchar *text;
   gchar *label;
} CompletionElement;

typedef struct {
   gchar    *value;
   GList    *elements;
} CompletionGroup;

typedef struct {
   GList*   groups;
} Completion;

typedef struct {
   gchar*      command;
   gboolean    is_group;
   GtkWidget*  row;
} CompletionRow;

// Completion Command
Completion*       cc_commands(gchar*);
Completion*       cc_bookmarks(gchar*);
Completion*       cc_quickmarks(gchar*);
Completion*       cc_open(gchar*);
Completion*       cc_session(gchar*);
//Completion*       cc_settings(gchar*);
Completion*       cc_downloads(gchar*);

void run_completion(gint);

#endif
