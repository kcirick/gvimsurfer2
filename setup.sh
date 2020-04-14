#!/bin/bash

REFRESH=$1
GVIMSURFER_DIR=$HOME/.config/gvimsurfer2

if [ $REFRESH ]; then
   echo "Starting all over again"
   echo "Removing directory $GVIMSURFER_DIR..."
   rm -rf $GVIMSURFER_DIR
fi

echo

if [ ! -d $GVIMSURFER_DIR ]; then
   echo "Making directory $GVIMSURFER_DIR..."
   mkdir -p $GVIMSURFER_DIR
else
   echo "$GVIMSURFER_DIR already exists!"
fi

if [ ! -f $GVIMSURFER_DIR/bookmarks ]; then
   echo "Making empty file bookmarks"
   touch $GVIMSURFER_DIR/bookmarks
fi
if [ ! -f $GVIMSURFER_DIR/cookies ]; then
   echo "Making empty file cookies"
   touch $GVIMSURFER_DIR/cookies
fi
if [ ! -f $GVIMSURFER_DIR/history ]; then
   echo "Making empty file history"
   touch $GVIMSURFER_DIR/history
fi

if [ ! -f $GVIMSURFER_DIR/configrc ]; then
   echo "Copying configuration file (configrc) to $GVIMSURFER_DIR..."
   cp configrc $GVIMSURFER_DIR
fi

if [ ! -f $GVIMSURFER_DIR/ui.css ]; then
   echo "Copying UI CSS file (ui.css) to $GVIMSURFER_DIR..."
   cp ui.css $GVIMSURFER_DIR
fi

#if [ ! -f $GVIMSURFER_DIR/script.js ]; then
#   echo "Copying javascript file (script.js) to $GVISURFER_DIR..."
#   cp script.js $GVIMSURFER_DIR
#fi

echo "All Done!"

