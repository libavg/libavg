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
#
# Original author of this file is Robert Parcus <betoparcus@gmail.com>


from libavg import avg, app


class VSyncDiv(app.MainDiv):
    def onInit(self):
        self.__line = avg.LineNode(color='FFFFFF', parent=self)
        self.__x = 0

    def onFrame(self):
        self.__x += 1
        if self.__x == self.width:
            self.__x = 0
        self.__line.pos1 = (self.__x, 0)
        self.__line.pos2 = (self.__x, self.height)


if __name__ == '__main__':
    app.App().run(VSyncDiv(), app_resolution='800x600')

