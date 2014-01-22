#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de
#
# Original author of this file is OXullo Interecans <x at brainrapers dot org>
# Sponsored by Archimedes Exhibitions GmbH ( http://www.archimedes-exhibitions.de )


import libavg
from libavg import app


class MyScene(app.MainDiv):
    def onInit(self):
        # This message is displayed immediately
        app.flashmessage.FlashMessage('helloworld! this message will disappear in a bit')

        # Displayed when the 'v' key is pressed, again a simple flash message
        app.keyboardmanager.bindKeyDown('v',
                lambda: app.flashmessage.FlashMessage('v key pressed'), 'Test me')
        
        # This message shows the message in a different color and sends the text to
        # the logger as well
        app.keyboardmanager.bindKeyDown('b',
                lambda: app.flashmessage.FlashMessage('this is an error', isError=True),
                'Test me too')
        
        # This message disappears when it's acknowledged with a mouse click
        app.keyboardmanager.bindKeyDown('n',
                lambda: app.flashmessage.FlashMessage('persistent, click here to dismiss',
                        acknowledge=True),
                'Test me too again')
        
        libavg.avg.WordsNode(parent=self, pos=(10, 50), fontsize=20,
                text='Press the keys: v, b, n')


if __name__ == '__main__':
    app.App().run(MyScene())

