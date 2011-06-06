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

from libavg import avg

g_player = avg.Player.get()

class TouchVisualization(avg.DivNode):
    '''Visualisation Class for Touch and Track Events'''
    def __init__(self, event, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.cursorid = event.cursorid
        self.pos = avg.Point2D(event.pos)
        self.positions = [self.pos]
        radius = event.majoraxis.getNorm() if event.majoraxis.getNorm() > 20.0 else 20.0

        if event.source == avg.TOUCH:
            color = 'e5d8d8'
        else:
            color = 'd8e5e5'
        self.__transparentCircle= avg.CircleNode(r=radius+20,
                fillcolor=color,
                fillopacity=0.2,
                opacity=0.0,
                strokewidth=1,
                sensitive=False,
                parent=self)
        self.__pulsecircle = avg.CircleNode(r=radius,
                fillcolor=color,
                color=color,
                fillopacity=0.5,
                opacity=0.5,
                strokewidth=1,
                sensitive=False,
                parent=self)
        self.__majorAxis = avg.LineNode(pos1=(0,0),
                pos2=event.majoraxis,
                color="FFFFFF",
                sensitive=False,
                parent=self)
        self.__minorAxis = avg.LineNode(pos1=(0,0),
                pos2=event.minoraxis,
                color="FFFFFF",
                sensitive=False, parent=self)
        fontPos = (self.__pulsecircle.r, 0)
        avg.WordsNode(pos=fontPos,
                text="<br/>".join([str(event.source),str(self.cursorid)]),
                parent=self)
        self.line = avg.PolyLineNode(self.positions,
                color=color,
                parent=kwargs['parent'])
        pulseCircleAnim = avg.LinearAnim(self.__pulsecircle, 'r', 200, 50, radius)
        pulseCircleAnim.start()

    def __del__(self):
        self.line.unlink(True)

    def move(self, event):
        self.pos = event.pos
        self.positions.append(self.pos)
        if len(self.positions) > 100:
            self.positions.pop(0)
        radius = event.majoraxis.getNorm() if event.majoraxis.getNorm() > 20.0 else 20.0
        self.__pulsecircle.r = radius
        self.__majorAxis.pos2 = event.majoraxis
        self.__minorAxis.pos2 = event.minoraxis
        self.line.pos = self.positions


class TouchVisualizationOverlay(avg.DivNode):
    def __init__(self, **kwargs):
        super(TouchVisualizationOverlay, self).__init__(**kwargs)
        self.sensitive = False
        self.elementoutlinecolor='FFFFAA'

        self.__touchVisElements = {}

        rootNode = g_player.getRootNode()
        avg.RectNode(parent=self, size=self.size,
                fillopacity=0.2, fillcolor='000000')
        rootNode.connectEventHandler(avg.CURSORUP, avg.TOUCH | avg.TRACK,
                self, self.__onTouchUp)
        rootNode.connectEventHandler(avg.CURSORDOWN, avg.TOUCH | avg.TRACK,
                self, self.__onTouchDown)
        rootNode.connectEventHandler(avg.CURSORMOTION, avg.TOUCH | avg.TRACK,
                self, self.__onTouchMotion)
    
    def deinit(self):
        rootNode = g_player.getRootNode()
        rootNode.disconnectEventHandler(self, self.__onTouchDown)
        rootNode.disconnectEventHandler(self, self.__onTouchUp)
        rootNode.disconnectEventHandler(self, self.__onTouchMotion)

    def __onTouchDown(self, event):
        touchVis = TouchVisualization(event, parent=self)
        self.__touchVisElements[event.cursorid] = touchVis

    def __onTouchUp(self, event):
        if event.cursorid in self.__touchVisElements:
            self.__touchVisElements[event.cursorid].unlink(True)
            self.__touchVisElements[event.cursorid] = None
            del self.__touchVisElements[event.cursorid]

    def __onTouchMotion(self, event):
        if event.cursorid in self.__touchVisElements:
            self.__touchVisElements[event.cursorid].move(event)


class KeysCaptionNode(avg.DivNode):
    def __init__(self, **kwargs):
        super(KeysCaptionNode, self).__init__(**kwargs)
        self.sensitive = False
        self.opacity = 0
        
        self.__background = avg.RectNode(fillcolor='000000', fillopacity=0.6,
                opacity=0, size=(450, 450), parent=self)
                
        # self.__stackOfBackups = []
        # self.__stackKeyDown = []
        # self.__stackKeyUp = []
        # self.__stackUnicodeDown = []
        # self.__stackUnicodeUp = []
        
        self.__keysNode = avg.WordsNode(pos=(10, 10), fontsize=18,
                color='DDDDDD', parent=self)
        
        self.__isShown = False
       
    def toggleHelp(self):
        self.__isShown = not self.__isShown
        
        self.__keyBindDown = g_kbManager.getKeys('key', 'down')
        self.__keyBindUp = g_kbManager.getKeys('key', 'up')
        self.__keycodeBindDown = g_kbManager.getKeys('unicode', 'down')
        self.__keycodeBindUp = g_kbManager.getKeys('unicode', 'up')
        
        if self.__isShown:
            helpText = '<span><b>   ACTIVE KEYS </b><br/></span>'
            for key in sorted(self.__keyBindDown.iterkeys()):
                funcName = self.__keyBindDown[key][1]
                helpText = helpText + \
                    '<span><b>%s</b>     <small>%s</small><br/></span>'% (key, funcName)
            
            for key in sorted(self.__keyBindUp.iterkeys()):
                if not(key in self.__keyBindDown):
                    funcName = self.__keyBindUp[key][1]
                    helpText = helpText + \
                        '<span><b>%s</b>     ' + \
                        '<small>%s</small><br/></span>'% (key, funcName)
            
            for key in sorted(self.__keycodeBindDown.iterkeys()):
                funcName = self.__keycodeBindDown[key][1]
                helpText = helpText + \
                    '<span><b>%s</b>     <small>%s</small><br/></span>'% (key, funcName)
            
            for key in sorted(self.__keycodeBindUp.iterkeys()):
                if key in self.__keycodeBindDown:
                    pass
                else:
                    funcName = self.__keycodeBindUp[key][1]
                    helpText = helpText + \
                        '<span><b>%s</b>     ' + \
                        '<small>%s</small><br/></span>'% (key, funcName)
            
            self.__keysNode.text = helpText
            self.opacity = 1
            self.__background.size = self.__keysNode.getMediaSize()
            
            self.getParent().reorderChild(
                    self.getParent().indexOf(self), 
                    self.getParent().getNumChildren()-1) 
        else:
            self.__keysNode.text = ''
            self.opacity = 0
            
    # def backupKeys(self):
    #     # backup keybindings if you change active app.
    #     # keys will be stored in a dict. keybindings from new function can be loaded
    #     # and conflicts by double keybindings are reduced to only the active app.
    #     
    #     # self.showHelp()
    #     
    #     self.__stackKeyDown.append(g_kbManager.getKeys('key', 'down'))
    #     self.__stackKeyUp.append(g_kbManager.getKeys('key', 'up'))
    #     self.__stackUnicodeDown.append(g_kbManager.getKeys('unicode','down'))
    #     self.__stackUnicodeUp.append(g_kbManager.getKeys('unicode','up'))
    #     
    #     g_kbManager.setKeys({}, 'key', 'down')
    #     g_kbManager.setKeys({}, 'key', 'up')
    #     g_kbManager.setKeys({}, 'unicode', 'down')
    #     g_kbManager.setKeys({}, 'unicode', 'up')
    #     g_kbManager.bindUnicode('?', g_kbManager.activateHelp, 'HELP')
    #     
    # def restoreKeys(self):
    #     # restore keybindings if last active app will be active again.
    #     # stored keys will be restored in the keybindings.
    # 
    #     # self.showHelp()  
    #     
    #     g_kbManager.setKeys(self.__stackKeyDown.pop(), 'key', 'down')
    #     g_kbManager.setKeys(self.__stackKeyUp.pop(), 'key', 'up')
    #     g_kbManager.setKeys(self.__stackUnicodeDown.pop(), 'unicode', 'down')
    #     g_kbManager.setKeys(self.__stackUnicodeUp.pop(), 'unicode', 'up')


class KeyboardManager(object):
    _instance = None
    
    def __init__(self):
        if self._instance is not None:
            raise RuntimeError('KeyboardManager has been already instantiated')
            
        self.__keyBindDown = {}
        self.__keyBindUp = {}
        self.__unicodeBindDown = {}
        self.__unicodeBindUp = {}

        self.__onKeyDownCb = lambda e: False
        self.__onKeyUpCb = lambda e: False

        self.__keyCaptionsNode = None
        
        KeyboardManager._instance = self
    
    @classmethod
    def get(cls):
        if cls._instance is None:
            cls()
        
        return cls._instance
        
    def setup(self, onKeyDownCb, onKeyUpCb):
        rootNode = g_player.getRootNode()
        rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, self.__onKeyDown)
        rootNode.setEventHandler(avg.KEYUP, avg.NONE, self.__onKeyUp)
        
        self.__onKeyDownCb = onKeyDownCb
        self.__onKeyUpCb = onKeyUpCb

        self.__keyCaptionsNode = KeysCaptionNode(pos=(5,5), parent=rootNode)
        self.bindUnicode('?', self.__keyCaptionsNode.toggleHelp, 'HELP')
    
    def teardown(self):
        self.__keyBindDown = {}
        self.__keyBindUp = {}
        self.__unicodeBindDown = {}
        self.__unicodeBindUp = {}
        
        self.__keyCaptionsNode.unlink(True)
        del self.__keyCaptionsNode
        self.__keyCaptionsNode = None
        
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
        elif key in self.__keyBindUp:
            del self.__keyBindUp[key]
        elif key in self.__unicodeBindDown:
            del self.__unicodeBindDown[key]
        elif key in self.__unicodeBindUp:
            del self.__unicodeBindUp[key]
        else:
            raise KeyError('Key %s not found' % key)

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
        if self.__onKeyDownCb(event):
            return

        elif event.keystring in self.__keyBindDown:
            self.__keyBindDown[event.keystring][0]()   
        elif self.__checkUnicode(event, self.__unicodeBindDown) == 1:
            self.__unicodeBindDown[str(unichr(event.unicode))][0]()
        elif self.__checkUnicode(event, self.__unicodeBindDown) == 2:
            self.__unicodeBindDown[unichr(event.unicode).encode("utf-8")][0]()

    def __onKeyUp(self, event):
        if self.__onKeyUpCb(event):
            return

        if event.keystring in self.__keyBindUp:
            if event.unicode == event.keycode:
                self.__keyBindUp[event.keystring][0]()
            elif event.unicode == 0:    #shift and ctrl
                self.__keyBindUp[event.keystring][0]()
        elif self.__checkUnicode(event, self.__unicodeBindUp) == 1:
            self.__unicodeBindUp[str(unichr(event.unicode))][0]()
        elif self.__checkUnicode(event, self.__unicodeBindUp) == 2:
            self.__unicodeBindUp[unichr(event.unicode).encode("utf-8")][0]()


g_kbManager = KeyboardManager.get()
