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


class KeysCaptionNode():
    def __init__(self, keyManager):
        self.__keyManager = keyManager
        self.__stackOfBackups = []      
        self.__stackKeyDown = []
        self.__stackKeyUp = []
        self.__stackUnicodeDown = []
        self.__stackUnicodeUp = []
        
        self.__rectNode = g_player.createNode("""
            <div id="RectNode" opacity="0" sensitive="False" x="5" y="10" size="(450,450)"> 
                <rect id="rectWhite" strokewidth="0" fillopacity="0.1" 
                        fillcolor="FFFFFF" pos="(5,5)" size="(450,450)"/>
                <rect id="rectBlack" strokewidth="0" fillopacity="0.6" 
                        fillcolor="000000" pos="(6,6)" size="(451,451)"/>
            </div>
        """)
        self.__keysNode = g_player.createNode("""
                    <words x="10" y="10" fontsize="18" opacity="1" 
                            color="DDDDDD" text=""/>
            """)
        rootNode = g_player.getRootNode()
        rootNode.appendChild(self.__rectNode)
        self.__rectNode.appendChild(self.__keysNode)
        
        self.__isShown = False
       
    def toggleHelp(self):
        self.__isShown = not self.__isShown
        
        self.__keyBindDown = self.__keyManager.getKeys('key', 'down')
        self.__keyBindUp = self.__keyManager.getKeys('key', 'up')
        self.__keycodeBindDown = self.__keyManager.getKeys('unicode', 'down')
        self.__keycodeBindUp = self.__keyManager.getKeys('unicode', 'up')
        
        
        TextHelp = ''   
        if self.__isShown:
            TextHelp = TextHelp + "<span><b>   ACTIVE KEYS </b><br/></span>"
            for key in sorted(self.__keyBindDown.iterkeys()):
                funcName = self.__keyBindDown[key][1]
                TextHelp = TextHelp + \
                    "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            for key in sorted(self.__keyBindUp.iterkeys()):
                if not(key in self.__keyBindDown):
                    funcName = self.__keyBindUp[key][1]
                    TextHelp = TextHelp + \
                        "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            for key in sorted(self.__keycodeBindDown.iterkeys()):
                funcName = self.__keycodeBindDown[key][1]
                TextHelp = TextHelp + \
                    "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            for key in sorted(self.__keycodeBindUp.iterkeys()):
                if key in self.__keycodeBindDown:
                    pass
                else:
                    funcName = self.__keycodeBindUp[key][1]
                    TextHelp = TextHelp + \
                        "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            self.__keysNode.text = TextHelp
            self.__rectNode.opacity = 1
            g_player.getElementByID('rectWhite').size = self.__keysNode.getMediaSize()
            g_player.getElementByID('rectBlack').size = self.__keysNode.getMediaSize()
            
            self.__rectNode.getParent().reorderChild(
                    self.__rectNode.getParent().indexOf(self.__rectNode), 
                    self.__rectNode.getParent().getNumChildren()-1) 
            
        else:
            self.__keysNode.text = ''
            self.__rectNode.opacity = 0
            
    # def backupKeys(self):
    #     # backup keybindings if you change active app.
    #     # keys will be stored in a dict. keybindings from new function can be loaded
    #     # and conflicts by double keybindings are reduced to only the active app.
    #     
    #     # self.showHelp()
    #     
    #     self.__stackKeyDown.append(self.__keyManager.getKeys('key', 'down'))
    #     self.__stackKeyUp.append(self.__keyManager.getKeys('key', 'up'))
    #     self.__stackUnicodeDown.append(self.__keyManager.getKeys('unicode','down'))
    #     self.__stackUnicodeUp.append(self.__keyManager.getKeys('unicode','up'))
    #     
    #     self.__keyManager.setKeys({}, 'key', 'down')
    #     self.__keyManager.setKeys({}, 'key', 'up')
    #     self.__keyManager.setKeys({}, 'unicode', 'down')
    #     self.__keyManager.setKeys({}, 'unicode', 'up')
    #     self.__keyManager.bindUnicode('?', self.__keyManager.activateHelp, 'HELP')
    #     
    # def restoreKeys(self):
    #     # restore keybindings if last active app will be active again.
    #     # stored keys will be restored in the keybindings.
    # 
    #     # self.showHelp()  
    #     
    #     self.__keyManager.setKeys(self.__stackKeyDown.pop(), 'key', 'down')
    #     self.__keyManager.setKeys(self.__stackKeyUp.pop(), 'key', 'up')
    #     self.__keyManager.setKeys(self.__stackUnicodeDown.pop(), 'unicode', 'down')
    #     self.__keyManager.setKeys(self.__stackUnicodeUp.pop(), 'unicode', 'up')


class KeyManager(object):
    def __init__(self, onKeyDownCb=lambda e: False, onKeyUpCb=lambda e: False):
        rootNode = g_player.getRootNode()
        rootNode.setEventHandler(avg.KEYDOWN, avg.NONE, self.__onKeyDown)
        rootNode.setEventHandler(avg.KEYUP, avg.NONE, self.__onKeyUp)
        
        self.__onKeyDownCb = onKeyDownCb
        self.__onKeyUpCb = onKeyUpCb
        
        self.__keyBindDown = {}
        self.__keyBindUp = {}
        self.__unicodeBindDown = {}
        self.__unicodeBindUp = {}

        self.__keyCaptionsNode = KeysCaptionNode(self)
        self.bindUnicode('?', self.__keyCaptionsNode.toggleHelp, 'HELP')
    
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

