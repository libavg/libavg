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

# Changes internal to libui-branch
sed -i 's/avg.Player.KEYUP/avg.Player.KEY_UP/g' *.py
sed -i 's/avg.Player.KEYDOWN/avg.Player.KEY_DOWN/g' *.py
sed -i 's/avg.Player.PLAYBACKSTART/avg.Player.PLAYBACK_START/g' *.py
sed -i 's/avg.Player.PLAYBACKEND/avg.Player.PLAYBACK_END/g' *.py

sed -i 's/avg.Node.CURSORMOTION/avg.Node.CURSOR_MOTION/g' *.py
sed -i 's/avg.Node.CURSORUP/avg.Node.CURSOR_UP/g' *.py
sed -i 's/avg.Node.CURSORDOWN/avg.Node.CURSOR_DOWN/g' *.py
sed -i 's/avg.Node.HOVERMOTION/avg.Node.HOVER_MOTION/g' *.py
sed -i 's/avg.Node.HOVERUP/avg.Node.HOVER_UP/g' *.py
sed -i 's/avg.Node.HOVERDOWN/avg.Node.HOVER_DOWN/g' *.py
sed -i 's/avg.Node.ENDOFFILE/avg.Node.END_OF_FILE/g' *.py

sed -i 's/avg.Contact.CURSORMOTION/avg.Contact.CURSOR_MOTION/g' *.py
sed -i 's/avg.Contact.CURSORUP/avg.Contact.CURSOR_UP/g' *.py

