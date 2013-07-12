#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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

from libavg import *


class TouchApp(app.MainDiv):
    def onArgvParsed(self, options, args, parser):
        if self.settings.getBoolean("app_fullscreen"):
            self.settings.set("app_resolution", "") # use screen resolution

    def onInit(self):
        self.subscribe(avg.Node.CURSOR_DOWN, self.__onDown)
        app.instance.debugPanel.toggleTouchVisualization()

    def __onDown(self, event):
#        if event.source == avg.MOUSE:
#            print event.type, event.button
#        else:
#            print event.type
        if (event.contact):
            event.contact.subscribe(avg.Contact.CURSOR_MOTION, self.__onContact)
            event.contact.subscribe(avg.Contact.CURSOR_UP, self.__onContact)
            contact = event.contact
#            print "new contact: ", contact.id, event.pos, contact.age, \
#                    contact.distancefromstart, contact.motionangle, contact.motionvec, \
#                    contact.distancetravelled

    def __onContact(self, event):
        contact = event.contact
#        print event.type, contact.id, event.pos, contact.age, \
#                contact.distancefromstart, contact.motionangle, contact.motionvec, \
#                contact.distancetravelled, event.speed


if __name__ == "__main__":
    app.App().run(TouchApp(), app_resolution="800x600", multitouch_enabled="true")

