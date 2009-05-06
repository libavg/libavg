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
# Original author of this file is Martin Heistermann <mh at sponc dot de>
#

import os
from AVGAppStarter import AVGAppStarter
from AVGMTAppStarter import AVGMTAppStarter

class AVGApp(object):
    multitouch = False
    def __init__(self, node):
        self.__isRunning = False

    def _enter(self):
        pass

    def _leave(self):
        pass

    def enter(self, onLeave = lambda: None):
        self.__isRunning = True
        self._onLeave = onLeave
        self._enter()

    def leave(self):
        self.__isRunning = False
        self._onLeave()
        self._leave()

    def onKey(self, event):
        """returns bool indicating if the event was handled
        by the application """
        return False

    def isRunning(self):
        return self.__isRunning

    @classmethod
    def start(cls, *args, **kwargs):
        if cls.multitouch and os.getenv("AVG_DEPLOY"):
            starter = AVGMTAppStarter
        else:
            starter = AVGAppStarter
        return starter(appClass = cls, *args, **kwargs)

