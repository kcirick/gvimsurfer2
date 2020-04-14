#ifndef SHORTCUTS_H
#define SHORTCUTS_H

typedef struct {
   guint       mask;
   gchar       *regex;
   guint       key;
   void        (*function)(Argument*);
   Argument    argument;
} Shortcut;

void sc_abort(Argument*);
void sc_focus_inputbar(Argument*);
void sc_search(Argument*);


#endif
