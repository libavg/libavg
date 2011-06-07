#!/usr/bin/env python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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
#

from libavg import *

g_Player = avg.Player.get()

class VSyncApp(AVGApp):
    
    def init(self):
        self.__line = avg.LineNode(pos1=(0,0), pos2=(0,1199), color="FFFFFF",
                parent=self._parentNode)
        self.__x = 0
        g_Player.setOnFrameHandler(self.onFrame)

    def onFrame(self):
        self.__x += 1
        if self.__x == 800:
            self.__x = 0
        self.__line.pos1 = (self.__x, 0)
        self.__line.pos2 = (self.__x, 599)
        
VSyncApp.start(resolution=(800,600))
