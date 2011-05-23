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

from libavg import avg, Point2D
import graph
from mtemu import MTemu
from AVGAppStarterHelp import MThelp

try:
    from alib.clicktest import ClickTest
except ImportError:
    ClickTest = None


g_player = avg.Player.get()
g_log = avg.Logger.get()


class TouchVisualization(avg.DivNode):
    def __init__(self, event, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.cursorid = event.cursorid
        self.pos = event.pos
        radius = event.majoraxis.getNorm()
        self.__circle = avg.CircleNode(r=radius, fillcolor="FFFFFF", fillopacity=0.5,
                strokewidth=0, parent=self)
        self.__majorAxis = avg.LineNode(pos1=(0,0), pos2=event.majoraxis, color="FFFFFF",
                parent=self)
        self.__minorAxis = avg.LineNode(pos1=(0,0), pos2=event.minoraxis, color="FFFFFF",
                parent=self)

    def move(self, event):
        self.pos = event.pos
        self.__circle.r = event.majoraxis.getNorm()+1
        self.__majorAxis.pos2 = event.majoraxis
        self.__minorAxis.pos2 = event.minoraxis


class AVGAppStarter(object):
    """Starts an AVGApp"""
    def __init__(self, appClass, resolution, debugWindowSize = None):
        self._AppClass = appClass
        resolution = Point2D(resolution)
        testMode = self._AppClass.avg_deploy == None
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

        self.__showMTEvents = False
        self.__touchViss = {} 
        self.__touchVisOverlay = avg.DivNode(sensitive=False, size=resolution,
                parent=rootNode)

        g_player.showCursor(testMode)

        if self._AppClass.fakeFullscreen:
            fullscreen = False
        else:
            fullscreen = not testMode

        g_player.setResolution(
                fullscreen,
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
        
        self.bindKey('o', self.__dumpObjects, 'Dump objects')
        self.bindKey('m', self.__showMemoryUsage, 'Show memory usage')
        self.bindKey('f', self.__showFrameRateUsage, 'Show frameTime usage')
        self.bindKey('.', self.__switchClickTest, 'Start clicktest')
        self.bindKey('t', self.__switchMtemu, 'Activate multitouch emulation')  
        self.bindKey('e', self.__switchShowMTEvents, 'Show multitouch events')  
        self.bindKey('s', self.__screenshot, 'Take screenshot')  
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
        self._appInstance.exit()

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
        handledByApp = self._activeApp.onKeyDown(event)
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
        handledByApp = self._activeApp.onKeyUp(event)
        if handledByApp:
            return
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
            self.__memGraph = graph.MemGraph("Memory Graph",
                    getValue = avg.getMemoryUsage)         
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
            self.__frGraph = graph.FrameRateGraph("FrameTime Graph",
                    getValue = g_player.getFrameTime)
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
   
    def __switchShowMTEvents(self):
        self.__showMTEvents = not(self.__showMTEvents)
        if self.__showMTEvents:
            self.__oldEventHook = g_player.getEventHook()
            g_player.setEventHook(self.__showMTEventHook)
        else:
            g_player.setEventHook(self.__oldEventHook)
            for id, touchVis in self.__touchViss.items():
                touchVis.unlink(True)
            self.__touchViss = {}

    def __showMTEventHook(self, event):
        if (isinstance(event, avg.TouchEvent) and event.source == avg.TOUCH and
                (event.type == avg.CURSORDOWN or event.type == avg.CURSORMOTION or
                 event.type == avg.CURSORUP)):
            try:
                touchVis = self.__touchViss[event.cursorid]
            except KeyError:
                touchVis = TouchVisualization(event, 
                        parent=self.__touchVisOverlay)
                self.__touchViss[event.cursorid] = touchVis
            if event.type == avg.CURSORDOWN:
                pass
            elif event.type == avg.CURSORMOTION:
                touchVis.move(event)
            elif event.type == avg.CURSORUP:
                touchVis.unlink(True)
                del self.__touchViss[event.cursorid]
        if self.__oldEventHook:
            return self.__oldEventHook()
        else:
            return False

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
    

