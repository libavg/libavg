#!/bin/bash

# Replaces old message enums with new ones.
sed -i 's/avg.KEYUP/avg.Event.KEY_UP/g' *.py
sed -i 's/avg.KEYDOWN/avg.Event.KEY_DOWN/g' *.py
sed -i 's/avg.CURSORMOTION/avg.Event.CURSOR_MOTION/g' *.py
sed -i 's/avg.CURSORUP/avg.Event.CURSOR_UP/g' *.py
sed -i 's/avg.CURSORDOWN/avg.Event.CURSOR_DOWN/g' *.py
sed -i 's/avg.CURSOROVER/avg.Event.CURSOR_OVER/g' *.py
sed -i 's/avg.CURSOROUT/avg.Event.CURSOR_OUT/g' *.py

sed -i 's/avg.MOUSE/avg.Event.MOUSE/g' *.py
sed -i 's/avg.TOUCH/avg.Event.TOUCH/g' *.py
sed -i 's/avg.TRACK/avg.Event.TRACK/g' *.py
sed -i 's/avg.CUSTOM/avg.Event.CUSTOM/g' *.py
sed -i 's/avg.NONE/avg.Event.NONE/g' *.py
