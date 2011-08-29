#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  libavg - Media Playback Engine. 
# Copyright (C) 2003-2011 Ulrich von Zadow
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  Current versions can be found at www.libavg.de

from libavg import avg, AVGApp
from libavg.avg import ImageNode

g_Player = avg.Player.get()

class HSL(AVGApp):

    multiTouch = False

    def init(self):
        rootNode = g_Player.getRootNode()

        background = ImageNode(parent=rootNode, href='../graphics/testfiles/hsl.png',
                size=(640,640))
        self.hsl_fx = avg.HueSatFXNode()
        background.setEffect(self.hsl_fx)
        self.sat = avg.WordsNode(pos=(800, 200), text="Sat: " +
                str(self.hsl_fx.saturation), parent=rootNode)
        self.light = avg.WordsNode(pos=(800, 250), text= "Bright" +
                str(self.hsl_fx.lightness), parent=rootNode)
        self.colorize = avg.WordsNode(pos=(800, 300), text="Colorized: " +
                str(self.hsl_fx.colorize), parent=rootNode)
        self.hue = avg.WordsNode(pos=(800, 350), text="Hue: " +
                str(self.hsl_fx.hue), parent=rootNode)
        rootNode.connectEventHandler(avg.KEYDOWN, avg.NONE, self,
                self.__onKeyDown)
        g_Player.setOnFrameHandler(self._increaseHue)

    def __onKeyDown(self, event):
        if event.keystring == 'v':
            self.hsl_fx.saturation -= 10
        elif event.keystring == 'b':
            self.hsl_fx.saturation += 10
        elif event.keystring == 'n':
            self.hsl_fx.lightness += 10
        elif event.keystring == 'k':
            self.hsl_fx.lightness -= 10
        elif event.keystring == 'h':
            self.hsl_fx.hue += 10
        elif event.keystring == 'g':
            self.hsl_fx.hue -= 10
        elif event.keystring == 'c':
            self.hsl_fx.colorize = not(self.hsl_fx.colorize)
        self.sat.text = "Sat: " + str(self.hsl_fx.saturation)
        self.light.text = "Bright: " + str(self.hsl_fx.lightness)
        self.colorize.text = "Colorized: " + str(self.hsl_fx.colorize)
        self.hue.text = "Hue: " + str(self.hsl_fx.hue)

    def _increaseHue(self):
        self.hsl_fx.hue += 1
        self.hue.text = "Hue: " + str(self.hsl_fx.hue)

if __name__ == '__main__':
    HSL.start(resolution=(1000,800),debugWindowSize=(800,600))

