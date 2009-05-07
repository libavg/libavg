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

class AVGApp(object):
    multitouch = False
    def __init__(self, parentNode):
        """initialization before Player.play()
        Use this only when needed, e.g. for
        Words.addFontDir(). Do not forget to call
        super(YourApp, self).__init__(parentNode)"""
        self.__isRunning = False
        self._parentNode = parentNode

    def init(self):
        """main initialization
        build node hierarchy under self.__parentNode."""
        pass

    def _enter(self):
        """enter the application, internal interface.
        override this and start all animations, intervals
        etc. here"""
        pass

    def _leave(self):
        """leave the application, internal interface.
        override this and stop all animations, intervals
        etc. Take care your application does not use any
        non-needed resources after this."""
        pass

    def enter(self, onLeave = lambda: None):
        """enter the application, external interface.
        Do not override this."""
        self.__isRunning = True
        self._onLeave = onLeave
        self._enter()

    def leave(self):
        """leave the application, external interface.
        Do not override this."""
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
        from AVGAppStarter import AVGAppStarter
        from AVGMTAppStarter import AVGMTAppStarter
        if cls.multitouch and os.getenv("AVG_DEPLOY"):
            starter = AVGMTAppStarter
        else:
            starter = AVGAppStarter
        return starter(appClass = cls, *args, **kwargs)

