# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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

from libavg import avg

class ScrollPane(avg.DivNode):

    def __init__(self, contentDiv, *args, **kwargs):

        avg.DivNode.__init__(self, crop=True, *args, **kwargs)
        self.appendChild(contentDiv)
        self.__contentDiv = contentDiv

    def setContentPos(self, pos):

        def constrain(pos, limit):
            if limit < 0:
                # Content larger than container
                if pos < limit:
                    pos = limit
                elif pos > 0:
                    pos = 0
            else:
                # Content smaller than container
                if pos < 0:
                    pos = 0
                elif pos > limit:
                    pos = limit
            return pos

        maxPos = self.getMaxContentPos()
        pos = avg.Point2D(pos)
        pos.x = constrain(pos.x, maxPos.x)
        pos.y = constrain(pos.y, maxPos.y)
        self.__contentDiv.pos = pos

    def getContentPos(self):
        return self.__contentDiv.pos
    contentpos = property(getContentPos, setContentPos)

    def getMaxContentPos(self):
        maxPos = self.size - self.__contentDiv.size
        return maxPos
