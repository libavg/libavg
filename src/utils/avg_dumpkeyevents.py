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


from libavg import app, player


class DumpKeyEvents(app.MainDiv):
    def onInit(self):
        player.subscribe(player.KEY_DOWN, self.onKeyDown)
        player.subscribe(player.KEY_UP, self.onKeyUp)

    def onKeyDown(self, event):
        self.__showMsg("KEY_DOWN", event)

    def onKeyUp(self, event):
        self.__showMsg("KEY_UP", event)

    def __showMsg(self, title, event):
        msg = (title + ": name='" + event.keyname + "', text='" + event.text +
                "', scancode=" + str(event.scancode))
        app.flashmessage.FlashMessage(msg)
        print msg

if __name__ == '__main__':
    app.App().run(DumpKeyEvents(), app_resolution='800x600')

