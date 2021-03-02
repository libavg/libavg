#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2012-2021 Ulrich von Zadow
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

from libavg import avg, gesture, app
import libavg

class TextRect(avg.DivNode):
    def __init__(self, text, parent=None, **kwargs):
        super().__init__(**kwargs)
        self.registerInstance(self, parent)
        self.rect = avg.RectNode(size=self.size, fillopacity=1, fillcolor="000000", 
                color="FFFFFF", parent=self)
        self.words = avg.WordsNode(color="FFFFFF", text=text, alignment="center", 
                parent=self)
        self.words.pos = (self.size-(0,self.words.size.y)) / 2

    def getSize(self):
        return self.__divSize

    def setSize(self, size):
        self.rect.size = size
        self.words.pos = (size-(0,self.words.size.y)) / 2
        self.__divSize = size
    __divSize = avg.DivNode.size
    size = property(getSize, setSize)


class SubclassDemo(app.MainDiv):

    def onInit(self):
        self.rect = TextRect(text="Hello World", pos=(20,20), size=(200,120), 
                parent=self)
        self.__recognizer = gesture.TapRecognizer(node=self.rect, 
                detectedHandler=self.onTap)
        
    def onTap(self):
        self.rect.size = self.rect.size + (10,10)

if __name__ == '__main__':
    app.App().run(SubclassDemo(), app_resolution='800x600')
