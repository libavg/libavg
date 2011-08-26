#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  libavg - Media Playback Engine. 
#  Copyright (C) 2003-2011 Ulrich von Zadow
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

class INVERT(AVGApp):

    multiTouch = False

    def init(self):
        rootNode = g_Player.getRootNode()

        orig = ImageNode(parent = rootNode, href='../graphics/testfiles/hsl.png')
        invert = ImageNode(parent = rootNode, href='../graphics/testfiles/hsl.png',
                pos=(orig.size.x+10, 0))
        invert.setEffect(avg.InvertFXNode())

if __name__ == '__main__':
    INVERT.start(resolution=(200,200),debugWindowSize=(200,200))

