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

from libavg import avg, Point2D
from mtemu import MTemu
from AVGAppStarterHelp import MThelp

try:
    from alib.clicktest import ClickTest
except ImportError:
    ClickTest = None


g_player = avg.Player.get()
g_log = avg.Logger.get()

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
            <div opacity="0" sensitive="False" x="10" y="10" size="%(size)s"> 
                <rect strokewidth="0" fillopacity="0.6" fillcolor="FFFFFF" 
                        size="%(size)s"/>
                <words x="10" y="%(wordsheight0)i" color="000080"/>
                <words x="10" y="%(wordsheight1)i" color="000080"/>
                <polyline color="008000"/>
                <polyline color="000080"/>
            </div>""" 
            % {'size': str(size), 'wordsheight0':size.y-22, 'wordsheight1':size.y-39})
        self.__graphSize = size-avg.Point2D(20, 20)
        rootNode.appendChild(self.__node)
        self.__textNode0 = self.__node.getChild(1)
        self.__textNode1 = self.__node.getChild(2)
        self.__maxLineNode = self.__node.getChild(3)
        self.__lineNode = self.__node.getChild(4)
        self.__interval = g_player.setInterval(1000, self.__nextSample)
        self.__sampleNum = 0
        avg.fadeIn(self.__node, 300)

    def delete(self):
        def kill():
            self.__node.unlink()
        avg.LinearAnim(self.__node, "opacity", 300, 1, 0, None, kill).start()
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
                
        self.__keyBindDown = {}
        self.__keyBindUp = {}
        rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, self.__onKeyDown)
        rootNode.setEventHandler(avg.KEYUP, avg.NONE, self.__onKeyUp)
        
        self.bindKey('o', self.__dumpObjects, 'dump objects')
        self.bindKey('m', self.__showMemoryUsage, 'show memory usage')
        self.bindKey('.', self.__switchClickTest, 'start clicktest')
        self.bindKey('t', self.__switchMtemu, 'activate multitouch emulation')  
        self.bindKey('?', self.activateHelp, 'HELP')  
        
        self.displayHelp = MThelp(self)
        self.showingHelp = False
        self.__showingMemGraph = False
        self.__runningClickTest = False
        self._initClickTest()
        self._mtEmu = None

        
        self._onBeforePlay()
        g_player.setTimeout(0, self._onStart)
        self._appInstance = self._AppClass(self._appNode)
        self._appInstance.setStarter(self)
        g_player.play()

    def _onBeforePlay(self):
        pass

    def _onStart(self):
        self._appInstance.init()
        self._appNode.opacity = 1
        self._appNode.sensitive = True
        self._activeApp = self._appInstance
        self._appInstance.enter()


    def _initClickTest(self):
        if ClickTest:
            self._clickTest = ClickTest(self._appNode, multiClick=False)
        else:
            self._clickTest = None
            
    def bindKey(self, key, func, funcName, state = 'Down'):
        if state == 'Down':   
            if key in self.__keyBindDown:
                raise KeyError # no double key bindings
            self.__keyBindDown[key] = (func, funcName)
        elif state == 'Up':
            if key in self.__keyBindUp:
                raise KeyError # no double key bindings
            self.__keyBindUp[key] = (func, funcName)
        else:
            raise KeyError 
            
    def unbindKey(self, key):
        del self.__keyBindDown[key]
        if key in self.__keyBindUp:
            del self.__keyBindUp[key]
    
    def getKeyDowns(self):
        return self.__keyBindDown
            
    def setKeyDowns(self, newkeyBindDown):
        self.__keyBindDown = newkeyBindDown
    
    def getKeyUps(self):
        return self.__keyBindUp
            
    def setKeyUps(self, newkeyBindUp):
        self.__keyBindUp = newkeyBindUp   
         
    def __unicodeInKeyBindings(self, event):
        for key in self.__keyBindDown.iterkeys():
            if unichr(event.unicode) == unicode(key):
                return True   
            else:
                pass
        return False
   
            
    def __onKeyDown(self, event):
        handledByApp = self._activeApp.onKey(event)
        if handledByApp:
            return
        elif event.keystring in self.__keyBindDown:
            if event.unicode == event.keycode:
                self.__keyBindDown[event.keystring][0]()
            elif event.unicode == 0:
                self.__keyBindDown[event.keystring][0]()
            return
        elif self.__unicodeInKeyBindings(event):          
            if event.unicode != event.keycode:
                self.__keyBindDown[str(unichr(event.unicode))][0]()
  
              
    def __onKeyUp(self, event):
        if event.keystring in self.__keyBindUp:
            self.__keyBindUp[event.keystring][0]()
            return
    
    def __dumpObjects(self):
        gc.collect()
        testHelper = g_player.getTestHelper()
        testHelper.dumpObjects()
        print "Num anims: ", avg.getNumRunningAnims()
        print "Num python objects: ", len(gc.get_objects()) 

    def __showMemoryUsage(self):
        if self.__showingMemGraph:
            self.__memGraph.delete()
            self.__memGraph = None
        else:
            self.__memGraph = MemGraph()
        self.__showingMemGraph = not(self.__showingMemGraph)
    
    def __switchClickTest(self):
        if self._clickTest:
            if self.__runningClickTest:
                g_log.trace(g_log.APP, 'Stopping clicktest')
                self._clickTest.stop()
            else:
                g_log.trace(g_log.APP, 'Starting clicktest')
                self._clickTest.start()
            
            self.__runningClickTest = not self.__runningClickTest

    def __switchMtemu(self):
        if self._mtEmu is None:
            self._mtEmu = MTemu()
            self.bindKey('left ctrl', self._mtEmu.changeMode, 'switch event mode')
            self.bindKey('right ctrl', self._mtEmu.changeMode, 'switch event mode')
            self.bindKey('left shift', self._mtEmu.multiTouch, 'create 2nd event')
            self.bindKey('right shift', self._mtEmu.multiTouch, 'create 2nd event')
            self.bindKey('left shift', self._mtEmu.multiTouch, 'create 2nd event', 'Up')
            self.bindKey('right shift', self._mtEmu.multiTouch, 'create 2nd event', 'Up')
            
        else:
            self.unbindKey('left ctrl')
            self.unbindKey('right ctrl')
            self.unbindKey('left shift')
            self.unbindKey('right shift')
            self._mtEmu.delete()
            self._mtEmu = None
    
    def activateHelp(self):
        if self.showingHelp == False:
            self.showingHelp = True
        else:
            self.showingHelp = False  
        self.displayHelp.showHelp()
    

