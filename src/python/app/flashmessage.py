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
# Original author of this file is OXullo Interecans <x at brainrapers dot org>
# Sponsored by Archimedes Exhibitions GmbH ( http://www.archimedes-exhibitions.de )


'''
Simple user notification API to report information to the user
'''

import libavg

class FlashMessage(object):
    DEFAULT_TIMEOUT = 3000
    LINE_HEIGHT = 20
    BORDER = 2
    
    messages = []
    
    @classmethod
    def remove(cls, killedMessage):
        cls.messages.remove(killedMessage)
        for index, message in enumerate(cls.messages):
            message.move(index)

    def __init__(self, text, timeout=DEFAULT_TIMEOUT, parent=None, isError=False,
            acknowledge=False):
        FlashMessage.messages.append(self)

        if parent is None:
            parent = libavg.player.getRootNode()

        if isError:
            color = 'ff0000'
        else:
            color = 'ffffff'

        rootNode = libavg.player.getRootNode()
        self.__container = libavg.avg.DivNode(sensitive=acknowledge, parent=parent)
        libavg.avg.RectNode(opacity=0, fillcolor='ffffff', fillopacity=1,
                pos=(self.BORDER, self.BORDER),
                size=(rootNode.size.x - self.BORDER * 2, self.LINE_HEIGHT - self.BORDER),
                parent=self.__container)
        libavg.avg.RectNode(opacity=0, fillcolor='000000', fillopacity=0.8,
                pos=(self.BORDER, self.BORDER),
                size=(rootNode.size.x - self.BORDER * 2, self.LINE_HEIGHT - self.BORDER),
                parent=self.__container)
        libavg.avg.WordsNode(text=text, fontsize=(self.LINE_HEIGHT - 3),
                sensitive=False,
                color=color,
                pos=(self.BORDER, self.BORDER),
                parent=self.__container)

        self.move(len(FlashMessage.messages) - 1, animate=False)

        if acknowledge:
            self.__container.subscribe(self.__container.CURSOR_DOWN,
                    lambda e: self.__kill())
        else:
            libavg.player.setTimeout(timeout, self.__kill)

    def move(self, index, animate=True):
        finalPos = (self.BORDER, index * self.LINE_HEIGHT)
        
        if animate:
            libavg.avg.LinearAnim(self.__container, 'pos', duration=150,
                    startValue=self.__container.pos,
                    endValue=finalPos).start()
        else:
            self.__container.pos = finalPos

    def __kill(self):
        def finalizeRemoval():
            self.__container.unlink(True)
            self.__container = None
            FlashMessage.remove(self)

        libavg.avg.fadeOut(self.__container, 200, finalizeRemoval)

