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
import math
import time

from libavg import avg, Point2D, anim

g_player = avg.Player.get()

class MemGraph():
    def __init__(self):
        self.__memGraphStartTime = g_player.getFrameTime()
        self.__usage = [0]
        self.__maxUsage = [0]
        self.__minutesUsage = [0]
        self.__minutesMaxUsage = [0]
        rootNode = g_player.getRootNode()
        size = avg.Point2D(rootNode.width-20, rootNode.height/6)
        self.__node = g_player.createNode("""
            <div opacity="0" sensitive="False" x="10" y="10" width="%(width)i" 
                    height="%(height)i">
                <rect strokewidth="0" fillopacity="0.6" fillcolor="FFFFFF" 
                        width="%(width)i" height="%(height)i"/>
                <words x="10" y="%(wordsheight0)i" color="000080"/>
                <words x="10" y="%(wordsheight1)i" color="000080"/>
                <polyline color="008000"/>
                <polyline color="000080"/>
            </div>""" 
            % {'width': size.x, 'height':size.y, 'wordsheight0':size.y-22, 
                    'wordsheight1':size.y-39})
        self.__graphSize = size-avg.Point2D(20, 20)
        rootNode.appendChild(self.__node)
        self.__textNode0 = self.__node.getChild(1)
        self.__textNode1 = self.__node.getChild(2)
        self.__maxLineNode = self.__node.getChild(3)
        self.__lineNode = self.__node.getChild(4)
        self.__interval = g_player.setInterval(1000, self.__nextSample)
        self.__sampleNum = 0
        anim.fadeIn(self.__node, 300)

    def delete(self):
        def kill():
            self.__node.unlink()
        anim.LinearAnim(self.__node, "opacity", 300, 1, 0, onStop=kill).start()
        g_player.clearInterval(self.__interval)

    def __nextSample(self):
        curUsage = avg.getMemoryUsage()
        self.__usage.append(curUsage)
        maxUsage = self.__maxUsage[-1]
        if curUsage>maxUsage:
            maxUsage = curUsage
            lastMaxChangeTime = time.time()
            self.__textNode1.text = ("Last increase in maximum: "
                    +time.strftime("%H:%M:%S", time.localtime(lastMaxChangeTime)))
        self.__maxUsage.append(maxUsage)
        self.__sampleNum += 1
        if self.__sampleNum % 60 == 0:
            lastMinuteAverage = sum(self.__usage[-60:])/60
            self.__minutesUsage.append(lastMinuteAverage)
            self.__minutesMaxUsage.append(maxUsage)

        if self.__sampleNum < 60*60:
            self.__plotLine(self.__usage, self.__lineNode, maxUsage)
            self.__plotLine(self.__maxUsage, self.__maxLineNode, maxUsage)
        else:
            self.__plotLine(self.__minutesUsage, self.__lineNode, maxUsage)
            self.__plotLine(self.__minutesMaxUsage, self.__maxLineNode, maxUsage)
        self.__textNode0.text = ("Max. memory usage: %(size).2f MB"
                %{"size":maxUsage/(1024*1024.)})

    def __plotLine(self, data, node, maxy):
        yfactor = self.__graphSize.y/float(maxy)
        xfactor = self.__graphSize.x/float(len(data)-1)
        node.pos = [(pos[0]*xfactor+10, (maxy-pos[1])*yfactor+10) 
                for pos in enumerate(data)]

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

        self._appNode = g_player.createNode('div',{
            'opacity': 0,
            'sensitive': False})
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

        self.bindKey('o', self.__dumpObjects)
        self.bindKey('m', self.__showMemoryUsage)
        self.__showingMemGraph = False

        self._onBeforePlay()
        g_player.setTimeout(0, self._onStart)
        self._appInstance = self._AppClass(self._appNode)
        g_player.play()

    def _onBeforePlay(self):
        pass

    def _onStart(self):
        self._appInstance.init()
        self._appNode.opacity = 1
        self._appNode.sensitive = True
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

    def __dumpObjects(self):
        gc.collect()
        testHelper = g_player.getTestHelper()
        testHelper.dumpObjects()
        print "Num anims: ", anim.getNumRunningAnims()
        print "Num python objects: ", len(gc.get_objects()) 

    def __showMemoryUsage(self):
        if self.__showingMemGraph:
            self.__memGraph.delete()
            self.__memGraph = None
        else:
            self.__memGraph = MemGraph()
        self.__showingMemGraph = not(self.__showingMemGraph)
