#ifndef CONFIG_BINDS_H
#define CONFIG_BINDS_H

//--- keyboard shortcuts ---
#define CMASK     GDK_CONTROL_MASK
#define SMASK     GDK_SHIFT_MASK
#define CSMASK    (CMASK | SMASK)
#define ALL_MASK  (CMASK | SMASK | GDK_MOD1_MASK)

Shortcut shortcuts[] = {
   // mask, regex          key,              function,            argument
   {0,      NULL,          GDK_KEY_Escape,   sc_abort,            { 0 } },
   {0,      NULL,          GDK_KEY_n,        sc_search,           { .b=FORWARD  } },
   {0,      NULL,          GDK_KEY_N,        sc_search,           { .b=BACKWARD } },
   {0,      NULL,          GDK_KEY_colon,    sc_focus_inputbar,   { .data=":" } },
   {0,      NULL,          GDK_KEY_slash,    sc_focus_inputbar,   { .data="/" } },
   {0,      NULL,          GDK_KEY_o,        sc_focus_inputbar,   { .data=":open "   } },
   {0,      NULL,          GDK_KEY_t,        sc_focus_inputbar,   { .data=":tabnew " } },
   {0,      NULL,          GDK_KEY_w,        sc_focus_inputbar,   { .data=":winnew " } },
   {0,      NULL,          GDK_KEY_L,        sc_navigate,         { .b=FORWARD  } },
   {0,      NULL,          GDK_KEY_H,        sc_navigate,         { .b=BACKWARD } },
   {0,      "^g$",         GDK_KEY_t,        sc_navigate_tabs,    { .i=NEXT     } },
   {0,      "^g$",         GDK_KEY_T,        sc_navigate_tabs,    { .i=PREVIOUS } },
   {0,      "^g$",         GDK_KEY_h,        sc_go_home,          { 0 } },
   {0,      "^[0-9]+q$",   GDK_KEY_m,        sc_quickmark,        { 0 } },
   {0,      0,             GDK_KEY_y,        sc_copy_uri,         { 0 } },
};

#endif
