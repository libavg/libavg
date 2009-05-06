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
import gc
from libavg import avg, Point2D, anim
g_player = avg.Player.get()

class AVGAppStarter(object):
    """Starts an AVGApp"""
    def __init__(self, appClass, resolution, debugWindowSize = None):
        self._AppClass = appClass
        resolution = Point2D(resolution)
        testMode = os.getenv("AVG_DEPLOY") == None
        if testMode and debugWindowSize is not None:
            debugWindowSize = Point2D(debugWindowSize)
        else:
            debugWindowSize = Point2D(0, 0)

        log = avg.Logger.get()
        log.setCategories(
                log.APP |
                log.WARNING |
                log.ERROR |
                log.PROFILE |
        #       log.PROFILE_LATEFRAMES |
                log.CONFIG  |
        #       log.MEMORY  |
        #       log.BLTS    |
        #       log.EVENTS  |
        #       log.EVENTS2  |
        0)

        width = int(resolution.x)
        height = int(resolution.y)
        # dynamic avg creation in order to set resolution
        g_player.loadString("""
<?xml version="1.0"?>
<!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
<avg width="%(width)u" height="%(height)u">
</avg>""" % {
            'width': width,
            'height': height,
            })
        rootNode = g_player.getRootNode()

        self._appNode = g_player.createNode('div',{})
        # the app should know the size of its "root" node:
        self._appNode.size = rootNode.size
        rootNode.appendChild(self._appNode)

        g_player.showCursor(testMode)
        g_player.setResolution(
                not testMode, # fullscreen
                int(debugWindowSize.x), int(debugWindowSize.y),
                0 # color depth
                )

        self.__keyBindings = {}
        rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, self.__onKey)

        def dumpObjects():
            gc.collect()
            testHelper = g_player.getTestHelper()
            testHelper.dumpObjects()
            print "Num anims: ", anim.getNumRunningAnims()
            print "Num python objects: ", len(gc.get_objects())
        self.bindKey('o', dumpObjects)

        self._onBeforePlay()
        g_player.setTimeout(0, self._onStart)
        g_player.play()

    def _onBeforePlay(self):
        pass

    def _onStart(self):
        self._appInstance = self._AppClass(self._appNode)
        self._activeApp = self._appInstance
        self._appInstance.enter()

    def bindKey(self, key, func):
        if key in self.__keyBindings:
            raise KeyError # no double key bindings
        self.__keyBindings[key] = func

    def unbindKey(self, key):
        del self.__keyBindings[key]

    def __onKey(self, event):
        handledByApp = self._activeApp.onKey(event)
        if handledByApp:
            return
        elif event.keystring in self.__keyBindings:
            self.__keyBindings[event.keystring]()
            return


