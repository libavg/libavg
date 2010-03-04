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


class Graph():
    def __init__(self, graph,getValue):
        self._getValue = getValue

        self._values = []
        self._xSkip = 2
        self._lastCurUsage = 0
        self._maxFrameTime = 0
        
        self._memGraphStartTime = g_player.getFrameTime()
        self._curUsage = 0
        self._usage = [0]
        self._maxUsage = [0]
        self._minutesUsage = [0]
        self._minutesMaxUsage = [0]
        self._rootNode = g_player.getRootNode()
        size = avg.Point2D(self._rootNode.width-20, self._rootNode.height/6)
        
        self._node = g_player.createNode("""
            <div opacity="0" sensitive="False" x="10" y="10" size="%(size)s"> 
                <rect strokewidth="0" fillopacity="0.6" fillcolor="FFFFFF" 
                        size="%(size)s"/>
                <words x="10" y="%(wordsheight0)i" color="000080"/>
                <words x="10" y="%(wordsheight1)i" color="000080"/>
                <polyline color="008000"/>
                <polyline color="000080"/>
                <words x="10" y="0" color="000080"/>
            </div>""" 
            % {'size': str(size), 'wordsheight0':size.y-22, 'wordsheight1':size.y-39})
        
        self._graphSize = size-avg.Point2D(20, 20)
        self._rootNode.appendChild(self._node)
        self._textNode0 = self._node.getChild(1)
        self._textNode1 = self._node.getChild(2)
        self._maxLineNode = self._node.getChild(3)
        self._lineNode = self._node.getChild(4)
        self.__graphText = self._node.getChild(5)
        self.__graphText.text = graph

        self._setup()
        self._sampleNum = 0
        avg.fadeIn(self._node, 300)
        
    def _setup(self):
        raise RuntimeError, 'Please overload _setup() function'
    
    def setYpos(self,ypos):
        self._node.y = ypos

    def delete(self):
        def kill():
            self._node.unlink()
        avg.LinearAnim(self._node, "opacity", 300, 1, 0, None, kill).start()
        g_player.clearInterval(self._interval)
        self._interval = None

    
        
class MemGraph(Graph):
    def _setup(self):
        self._interval = g_player.setInterval(1000, self._nextMemSample)

    def _nextMemSample(self):
        curUsage = self._getValue()
        self._usage.append(curUsage)
        maxUsage = self._maxUsage[-1]
        if curUsage>maxUsage:
            maxUsage = curUsage
            lastMaxChangeTime = time.time()
            self._textNode1.text = ("Last increase in maximum: "
                    +time.strftime("%H:%M:%S", time.localtime(lastMaxChangeTime)))
        self._maxUsage.append(maxUsage)
        self._sampleNum += 1
        if self._sampleNum % 60 == 0:
            lastMinuteAverage = sum(self._usage[-60:])/60
            self._minutesUsage.append(lastMinuteAverage)
            self._minutesMaxUsage.append(maxUsage)

        if self._sampleNum < 60*60:
            self._plotLine(self._usage, self._lineNode, maxUsage)
            self._plotLine(self._maxUsage, self._maxLineNode, maxUsage)
        else:
            self._plotLine(self._minutesUsage, self._lineNode, maxUsage)
            self._plotLine(self._minutesMaxUsage, self._maxLineNode, maxUsage)
        self._textNode0.text = ("Max. memory usage: %(size).2f MB"
                %{"size":maxUsage/(1024*1024.)})

    def _plotLine(self, data, node, maxy):
        yfactor = self._graphSize.y/float(maxy)
        xfactor = self._graphSize.x/float(len(data)-1)
        node.pos = [(pos[0]*xfactor+10, (maxy-pos[1])*yfactor+10) 
                for pos in enumerate(data)]

class FrameRateGraph(Graph):
    def _setup(self):
        self._interval = g_player.setOnFrameHandler(self._nextFrameTimeSample)         
           
    def _nextFrameTimeSample(self):       
        val = self._frameTimeSample()
        self._appendValue(val)
        self._sampleNum += 1
        
    def _appendValue(self,value):
        y = value + self._rootNode.height/6
        numValues = int(self._rootNode.width/self._xSkip)-10    
        self._values = (self._values + [y])[-numValues:]
        self._plotGraph()
        
    def _frameTimeSample(self):
        frameTime = self._getValue()  
        diff = frameTime - self._lastCurUsage       
        #if(self._sampleNum % 1800 == 0):
           # self._maxFrameTime = 0
        if(self._sampleNum<2):
            self._maxFrameTime = 0
        if(diff>self._maxFrameTime):
            lastMaxChangeTime = time.time()     
            self._maxFrameTime = diff
            self._textNode0.text = ("Max FrameTime: %.f" %self._maxFrameTime + " ms" + 
                "   Time: " +time.strftime("%H:%M:%S", time.localtime(lastMaxChangeTime)))
        if diff>self._node.y-1:
            y = self._node.y-1
            
        self._lastCurUsage = frameTime
        self._textNode1.text = ("Current FrameTime: %.f" %diff + " ms" )      
        return -diff

    def _plotGraph(self):
        self._lineNode.pos = self._getCoords()
        
    def _getCoords(self):
        return zip(xrange(10,len(self._values)*self._xSkip, self._xSkip), self._values)
    
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
        self.__graphs = 0
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
        self.__unicodeBindDown = {}
        self.__unicodeBindUp = {}
        
        self.__notifyNode = None
        
        rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, self.__onKeyDown)
        rootNode.setEventHandler(avg.KEYUP, avg.NONE, self.__onKeyUp)
        
        self.bindKey('o', self.__dumpObjects, 'dump objects')
        self.bindKey('m', self.__showMemoryUsage, 'show memory usage')
        self.bindKey('f', self.__showFrameRateUsage, 'show frameTime usage')
        self.bindKey('.', self.__switchClickTest, 'start clicktest')
        self.bindKey('t', self.__switchMtemu, 'activate multitouch emulation')  
        self.bindKey('s', self.__screenshot, 'take screenshot')  
        self.bindUnicode('?', self.activateHelp, 'HELP')  
        

        self.helpInstance = MThelp(self)
        self.showingHelp = False
        self.__showingMemGraph = False
        self.__showingFrGraph = False
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
            
    def bindKey(self, key, func, funcName, state = 'down'):
        if state == 'down':   
            if key in self.__keyBindDown:
                raise KeyError # no double key bindings
            self.__keyBindDown[key] = (func, funcName)
        elif state == 'up':
            if key in self.__keyBindUp:
                print key
                raise KeyError # no double key bindings
            self.__keyBindUp[key] = (func, funcName)
        else:
            raise KeyError 
            
    def unbindKey(self, key):
        if key in self.__keyBindDown:
            del self.__keyBindDown[key]
        if key in self.__keyBindUp:
            del self.__keyBindUp[key]
        if key in self.__unicodeBindDown:
            del self.__unicodeBindDown[key]
        if key in self.__unicodeBindUp:
            del self.__unicodeBindUp[key]    
    
    def bindUnicode(self, key, func, funcName, state = 'down'):
        if state == 'down':   
            if key in self.__unicodeBindDown:
                raise KeyError # no double key bindings
            self.__unicodeBindDown[key] = (func, funcName)
        elif state == 'up':
            if key in self.__unicodeBindUp:
                raise KeyError # no double key bindings
            self.__unicodeBindUp[key] = (func, funcName)
        else:
            raise KeyError    
    
    def getKeys(self, bindtype = 'key', action = 'down'):
        if bindtype == 'key':
            if action == 'down':
                return self.__keyBindDown
            elif action == 'up':
                return self.__keyBindUp
        elif bindtype == 'unicode':
            if action == 'down':
                return self.__unicodeBindDown
            elif action == 'up':
                return self.__unicodeBindUp
    
    def setKeys(self, newKeyBindings, bindtype = 'key', action = 'down'):
        if bindtype == 'key':
            if action == 'down':
                self.__keyBindDown = newKeyBindings
            elif action == 'up':
                self.__keyBindUp = newKeyBindings
        elif bindtype == 'unicode':
            if action == 'down':
                self.__unicodeBindDown = newKeyBindings
            elif action == 'up':
                self.__unicodeBindUp = newKeyBindings
    
    def __checkUnicode(self, event, Bindings):
        x = 0
        try:
            if str(unichr(event.unicode)) in Bindings: 
                x = 1
                return x
        except: 
            pass
        try:
            if unichr(event.unicode).encode("utf-8") in Bindings:
                x = 2
                return x
        except:
            pass
        return x
          
    def __onKeyDown(self, event):
        handledByApp = self._activeApp.onKey(event)
        if handledByApp:
            return
        elif event.keystring in self.__keyBindDown:
            self.__keyBindDown[event.keystring][0]()   
        elif self.__checkUnicode(event, self.__unicodeBindDown) == 1:
            self.__unicodeBindDown[str(unichr(event.unicode))][0]()
            return
        elif self.__checkUnicode(event, self.__unicodeBindDown) == 2:
            self.__unicodeBindDown[unichr(event.unicode).encode("utf-8")][0]()
            return
        
                        
    def __onKeyUp(self, event):
        if event.keystring in self.__keyBindUp:
            if event.unicode == event.keycode:
                self.__keyBindUp[event.keystring][0]()
                return
            elif event.unicode == 0:    #shift and ctrl
                self.__keyBindUp[event.keystring][0]()
        elif self.__checkUnicode(event, self.__unicodeBindUp) == 1:
            self.__unicodeBindUp[str(unichr(event.unicode))][0]()
            return
        elif self.__checkUnicode(event, self.__unicodeBindUp) == 2:
            self.__unicodeBindUp[unichr(event.unicode).encode("utf-8")][0]()
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
            self.__graphs = self.__graphs -1
            if(self.__graphs == 1 ):
                self.__frGraph.setYpos(10)
        else:
            self.__memGraph = MemGraph("Memory Graph", getValue = lambda: avg.getMemoryUsage())         
            self.__graphs = self.__graphs +1          
            if(self.__graphs > 1 ):
                self.__memGraph.setYpos(190)       
        self.__showingMemGraph = not(self.__showingMemGraph)
     
        
    def __showFrameRateUsage(self):
        if self.__showingFrGraph:
            self.__frGraph.delete()
            self.__frGraph = None
            self.__graphs = self.__graphs -1
            if(self.__graphs == 1 ):
                self.__memGraph.setYpos(10)
        else:      
            self.__frGraph = FrameRateGraph("FrameTime Graph",getValue = lambda: g_player.getFrameTime())
            self.__graphs = self.__graphs +1 
            if(self.__graphs >1):               
                self.__frGraph.setYpos(190)           
        self.__showingFrGraph = not(self.__showingFrGraph)
    
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
            self.bindKey('left shift', self._mtEmu.multiTouch, 'create 2nd event', 'up')
            self.bindKey('right shift', self._mtEmu.multiTouch, 'create 2nd event', 'up')
            
        else:
            self.unbindKey('left ctrl')
            self.unbindKey('right ctrl')
            self.unbindKey('left shift')
            self.unbindKey('right shift')
            self._mtEmu.delete()
            self._mtEmu = None
    
    def __killNotifyNode(self):
        if self.__notifyNode:
            self.__notifyNode.unlink()
            self.__notifyNode = None
            
    def __screenshot(self):
        fnum = 0
        fnameTemplate = 'screenshot-%03d.png'
        while os.path.exists(fnameTemplate % fnum):
            fnum += 1
        
        try:
            g_player.screenshot().save('screenshot-%03d.png' % fnum)
        except RuntimeError:
            text = 'Cannot save snapshot file'
        else:
            text='Screenshot saved as ' + fnameTemplate % fnum
        
        self.__killNotifyNode()
        
        self.__notifyNode = avg.WordsNode(
            text=text, x=g_player.getRootNode().width - 50,
            y=g_player.getRootNode().height - 50, alignment='right', fontsize=20,
            sensitive=False, parent=g_player.getRootNode())
            
        g_player.setTimeout(2000, self.__killNotifyNode)
        
    def activateHelp(self):
        if self.showingHelp == False:
            self.showingHelp = True
        else:
            self.showingHelp = False  
        self.helpInstance.showHelp()
    

