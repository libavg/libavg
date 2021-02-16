#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2016-2021 Ulrich von Zadow
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

from libavg import avg, app, sprites

class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.__spritesheet = sprites.Spritesheet("spritesheet.xml")
        self.__sprite1 = sprites.AnimatedSprite(self.__spritesheet, "Ball ", loop=True,
                pos=(100,100), parent=self)
        self.__sprite1.play()
        self.__sprite2 = sprites.AnimatedSprite(self.__spritesheet, "Ball2 ", loop=True,
                pos=(150,100), parent=self)
        self.__sprite2.play()

    def onExit(self):
        pass

app.App().run(MyMainDiv())

