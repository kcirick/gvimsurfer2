#ifndef CONFIG_BINDS_H
#define CONFIG_BINDS_H

//--- keyboard shortcuts ---
#define CMASK     GDK_CONTROL_MASK
#define SMASK     GDK_SHIFT_MASK
#define CSMASK    (CMASK | SMASK)
#define ALL_MASK  (CMASK | SMASK | GDK_MOD1_MASK)

Shortcut shortcuts[] = {
  // mask,  regex        key,                function,            argument
  {0,       NULL,        GDK_KEY_Escape,     sc_abort,            { 0 } },
  {0,       NULL,        GDK_KEY_n,          sc_search,           { .b=FORWARD  } },
  {0,       NULL,        GDK_KEY_N,          sc_search,           { .b=BACKWARD } },
  {0,       NULL,        GDK_KEY_colon,      sc_focus_inputbar,   { .data=":" } },
  {0,       NULL,        GDK_KEY_slash,      sc_focus_inputbar,   { .data="/" } }
};


#endif
