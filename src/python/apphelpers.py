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

import os

from libavg import avg, player

from app.touchvisualization import *


class KeysCaptionNode(avg.DivNode):
    def __init__(self, parent=None, **kwargs):
        super(KeysCaptionNode, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        
        self.sensitive = False
        self.opacity = 0
        
        self.__background = avg.RectNode(fillcolor='000000', fillopacity=0.6,
                opacity=0, size=(450, 450), parent=self)
        
        self.__keysNode = avg.WordsNode(pos=(10, 10), fontsize=18,
                color='DDDDDD', parent=self)
        
        self.__isShown = False
       
    def toggleHelp(self):
        self.__isShown = not self.__isShown
        
        keys = g_KbManager.getActiveKeyBindings()
        
        if self.__isShown:
            helpText = '<span><b>   ACTIVE KEYS </b><br/></span>'
            
            for keyObj in sorted(keys, key=lambda ko: ko.key):
                if keyObj.state == 'up':
                    stateAddition = ' (up)'
                else:
                    stateAddition = ''
                    
                helpText += ('<span><b>%s</b>    '
                        '<small>%s%s</small></span><br/>' % (keyObj.key,
                                keyObj.description, stateAddition))

            self.__keysNode.text = helpText
            self.opacity = 1
            self.__background.size = self.__keysNode.getMediaSize()
            
            self.getParent().reorderChild(
                    self.getParent().indexOf(self), 
                    self.getParent().getNumChildren()-1) 
        else:
            self.__keysNode.text = ''
            self.opacity = 0


class KeyBinding(object):
    def __init__(self, key, description, state, callback):
        if not isinstance(key, unicode) and not isinstance(key, str):
            raise TypeError('KeyBinding key should be either a string or unicode object')

        self.__key = key
        self.__description = description
        self.__state = state
        self.__callback = callback

    def __repr__(self):
        return '<%s key=%s (%s) state=%s>' % (self.__class__.__name__,
                self.__key, self.__description, self.__state)
    
    @property
    def key(self):
        return self.__key

    @property
    def description(self):
        return self.__description

    @property
    def state(self):
        return self.__state
        
    def checkKey(self, key, state):
        if state is not None and self.__state != state:
            return False
            
        return self.__key == key
        
    def checkEvent(self, event, state):
        if self.__state != state:
            return False

        if isinstance(self.__key, unicode):
            return self.__key == unichr(event.unicode)
        else:
            return self.__key == event.keystring
    
    def executeCallback(self):
        self.__callback()


class KeyboardManager(object):
    _instance = None
    TOGGLE_HELP_UNICODE = 63
    
    def __init__(self):
        if self._instance is not None:
            raise RuntimeError('KeyboardManager has been already instantiated')
        
        self.__keyBindings = []
        self.__keyBindingsStack = []

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
        player.subscribe(avg.Player.KEY_DOWN, self.__onKeyDown)
        player.subscribe(avg.Player.KEY_UP, self.__onKeyUp)
        
        self.__onKeyDownCb = onKeyDownCb
        self.__onKeyUpCb = onKeyUpCb

        self.__keyCaptionsNode = KeysCaptionNode(pos=(5,5), parent=player.getRootNode())
    
    def teardown(self):
        self.__keyBindings = []
        
        self.__keyCaptionsNode.unlink(True)
        del self.__keyCaptionsNode
        self.__keyCaptionsNode = None
    
    def push(self):
        self.__keyBindingsStack.append(self.__keyBindings)
        self.__keyBindings = []
    
    def pop(self):
        if not self.__keyBindingsStack:
            raise RuntimeError('Empty stack')
            
        self.__keyBindings = self.__keyBindingsStack.pop()
    
    def getActiveKeyBindings(self):
        return self.__keyBindings
        
    def bindKey(self, key, func, funcName, state='down'):
        import warnings
        warnings.warn('libavg.KeyboardManager is deprecated, use '
                'libavg.app.keyboardmanager instead')

        if isinstance(key, unicode) and state != 'down':
            raise RuntimeError('bindKey() with unicode keys '
                    'can be used only with state=down')

        if key == unichr(self.TOGGLE_HELP_UNICODE):
            raise RuntimeError('%s key is reserved')
            
        keyObj = self.__findKeyByKeystring(key, state)
        if keyObj is not None:
            raise RuntimeError('Key %s has already been bound (%s)' % (key, keyObj))
        
        self.__keyBindings.append(KeyBinding(key, funcName, state, func))
        
    def unbindKey(self, key):
        keyObj = self.__findKeyByKeystring(key)
        
        if keyObj is not None:
            self.__keyBindings.remove(keyObj)
        else:
            raise KeyError('Key %s not found' % key)

    def bindUnicode(self, key, func, funcName, state='down'):
        raise DeprecationWarning('Use bindKey() passing an unicode object as keystring')

    def __findKeyByEvent(self, event, state):
        for keyObj in self.__keyBindings:
            if keyObj.checkEvent(event, state):
                return keyObj
        
        return None
        
    def __findKeyByKeystring(self, key, state=None):
        for keyObj in self.__keyBindings:
            if keyObj.checkKey(key, state):
                return keyObj
        
        return None
        
    def __onKeyDown(self, event):
        if self.__onKeyDownCb(event):
            return
        elif event.unicode == self.TOGGLE_HELP_UNICODE:
            self.__keyCaptionsNode.toggleHelp()
        else:
            keyObj = self.__findKeyByEvent(event, 'down')
            if keyObj is not None:
                keyObj.executeCallback()
            
    def __onKeyUp(self, event):
        if self.__onKeyUpCb(event):
            return
        else:
            keyObj = self.__findKeyByEvent(event, 'up')
            if keyObj is not None:
                keyObj.executeCallback()


g_KbManager = KeyboardManager.get()
