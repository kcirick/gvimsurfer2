#ifndef CONFIG_COMMANDS_H
#define CONFIG_COMMANDS_H

static const Command commands[] = {
   // command,    function,               completion,       description
   {"back",       cmd_back,               0 },              // Go back in history
   {"forward",    cmd_forward,            0 },              // Go forward in history
   {"bmark",      cmd_bookmark,           0 },              // Add bookmark
//  {"download",  cmd_handle_downloads,   cc_downloads},    // List (or cancel) active downloads
//  {"qmark",     cmd_quickmark,          0 },              // Set quickmark
//  {"pmark",     cmd_pagemark,           0 },              // Set pagemark
   {"open",       cmd_open,               cc_open },        // Open URI in current tab
//  {"print",     cmd_print,              0 },              // Print page
   {"close",      cmd_close,              0 },              // Close current tab
   {"q",          cmd_close,              0 },              // Close current tab
   {"quit",       cmd_close,              0 },
   {"qall",       cmd_closeall,           0 },              // Close all tabs
   {"reload",     cmd_reload,             0 },              // Reload current page
//  {"saveas",   cmd_saveas,             0 },              // Save current document to disk
//  {"script",   cmd_load_script,        0 },              // Load a javascript file
//  {"session",  cmd_session,            cc_session },     // Save session with specified name
//  {"set",      cmd_settings,           cc_settings },    // Set an option
   {"stop",       cmd_stop,               0 },              // Stop loading the current page
   {"tabnew",     cmd_tabopen,            cc_open },        // Open URI in a new tab
   {"winnew",     cmd_winopen,            cc_open },        // Open URI in a new window
   {"write",      cmd_write,              0 },              // Write bookmark and history file
};


#endif

