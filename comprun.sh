#!/bin/bash

# In order to run this script, open a Terminal session
# to the same directory and type "chmod u+x comprun.sh"
# without the quotes once to update permissions. Then to
# run the script, from the same Terminal type "./comprun.sh"

# this worked for me on MacOS 12.4 Monterey, using SDL2 version 2.24.0

# exit script at first error
set -u -e

# run the makefile in the main subdirectory
cd main
make
make all_headers.h
cd ..

# Compile it and output to executable called 'app'
g++ -std=c++11 app.cpp GUISquare.cpp GUIPiece.cpp GUI.cpp main/main.o -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2 -o app

# clean up
cd main
make clean
cd ..

# runs the app
./app
