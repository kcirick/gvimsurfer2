# gVimSurfer2

A Web browser that looks and works like gVim (using webkit2gtk)

## Description:

  - Features/functions first inspired by jumanji and vimprobable
  - organized in a way that makes sense
  - Requires following as dependencies:
    - gtk+-3
    - webkit2gtk
  - All configuration is stored under $HOME/.config/gvimsurfer2 (run the setup script to create it - See installation section below)

## Features:

  - Look and feel:
    - Customize statusbar colours, completion colours and browser font
  - Navigate using keyboard
    - Tab completion for URI (bookmarks and history), commands and settings
  - Search engines
    - Can define multiple search engines through config file to search the web
  - Bookmarks
    - Store bookmarks (with tags) for future reference
  - Quickmarks (NEW)
    - Quickly mark a webpage you want to come back to. Not stored after the browser is closed
  - Sessions (NEW)
    - Save and load sessions (collections of tabs)
    - Last session is automatically saved

## Installation:

  1. Clone a copy: `> git clone https://github.com/kcirick/gvimsurfer2.git`
  2. Compile the code: `> make`
  3. Install: `> make install` (as root)
  4. Run the setup script: `> sh /usr/share/gvimsurfer/setup.sh`
  5. Make changes to configrc (located in $HOME/.config/gvimsurfer2)
  6. Run 'gvimsurfer2' (try 'gvimsurfer2 --help' for help)
  7. To uninstall: `> make uninstall` (as root)

## Screenshots:

   - To be added.


## Version Log:

  - 0.2 (Work in progress)
    - Add back quickmark feature
    - tabopen and winopen without url will open home page
    - Remove reference to external editor. Also remove code to view source
    - Remove tab number from title. Irrelevant information
    - Add debug mode to see debug message 
    - Backspace on buffer does what it's supposed to
    - Adding WebExtension support (needed for scrolling etc)

  - 0.1 (2020-04-13) (<a href='https://github.com/kcirick/gvimsurfer2/archive/v0.1.zip'>download .zip</a>): 
    - A working web browser
    - First port from gVimSurfer (webkit1/gtk2 version)

## To do / Known issues:

  - TODO: Code cleanup
  - TODO: Add more features
  - DONE: ~~Create a working browser~~

