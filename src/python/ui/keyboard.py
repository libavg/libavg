#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# libavg - Media Playback Engine.
# Copyright (C) 2003-2010 Ulrich von Zadow
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
# Original author of this module: Thomas Schott <scotty at c-base dot org>
#

import os.path

from libavg import avg

g_player = avg.Player.get()
g_logger = avg.Logger.get()


class Key(avg.ImageNode):
    def __init__(self, keyDef, ovlHref, onDownCallback, onUpCallback, sticky=False,
            *args, **kwargs):
        kwargs['pos'] = keyDef[1]
        kwargs['size'] = keyDef[2]
        kwargs['opacity'] = 0.0
        super(Key, self).__init__(*args, **kwargs)

        if ovlHref:
            self.__createImage(ovlHref)
        self.__keyCode = keyDef[0]
        self.__onDownCallback = onDownCallback
        self.__onUpCallback = onUpCallback
        self.__sticky = sticky
        if self.__sticky:
            self.__stickyIsDown = False
        self.__cursorID = None
        self.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, self.__onDown)
        self.setEventHandler(avg.CURSORUP, avg.MOUSE | avg.TOUCH, self.__onUpOut)
        self.setEventHandler(avg.CURSOROUT, avg.MOUSE | avg.TOUCH, self.__onUpOut)

    def reset(self):
        if self.__sticky:
            self.opacity = 0.0
            self.__stickyIsDown = False

    def __createImage(self, ovlHref):
        if os.path.isabs(ovlHref):
            effectiveHref = ovlHref
        else:
            effectiveHref = self.getParent().getEffectiveMediaDir() + ovlHref
        canvas = g_player.loadCanvasString(
        '''
            <canvas id="offscreen" size="%s">
                <image href="%s" pos="%s"/>
            </canvas>
        '''
        %(str(self.size), 
          effectiveHref,
          str(-self.pos)))
        canvas.render()
        self.setBitmap(canvas.screenshot())
        g_player.deleteCanvas('offscreen')

    def __onDown(self, event):
        if self.__sticky:
            self.__stickyIsDown = not(self.__stickyIsDown)
            if self.__stickyIsDown:
                self.__pseudoDown(event)
            else:
                self.__pseudoUp(event)
        else:
            if self.__cursorID:
                return
            self.__pseudoDown(event)

    def __onUpOut(self, event):
        if not self.__cursorID == event.cursorid:
            return
        if not (self.__sticky):
            self.__pseudoUp(event)

    def __pseudoDown(self, event):
        self.__cursorID = event.cursorid

        self.opacity = 1.0
        if self.__onDownCallback:
            self.__onDownCallback(event, self.__keyCode)
       
    def __pseudoUp(self, event):
        self.__cursorID = None

        self.opacity = 0.0
        if self.__onUpCallback:
            self.__onUpCallback(event, self.__keyCode)
        


class Keyboard(avg.DivNode):

    def __init__(self, bgHref, ovlHref, keyDefs, shiftKeyCode, altGrKeyCode=None,
            stickyShift=False, *args, **kwargs):
        # TODO: shift and altGr handling have some duplicated code.
        super(Keyboard, self).__init__(*args, **kwargs)

        self.__shiftKeyCode = shiftKeyCode
        self.__shiftDownCounter = 0
        self.__stickyShift = stickyShift
        self.__altGrKeyCode = altGrKeyCode
        self.__altGrKeyCounter = 0
        if not(self.__shiftKeyCode) and self.__altGrKeyCode:
            raise RuntimeError(
                    "Keyboard: If there is an altgr key, there must also be a shift key.")
        self.__codesPerKey = 1
        if self.__shiftKeyCode:
            self.__codesPerKey = 2
        if self.__altGrKeyCode:
            self.__codesPerKey = 3
        
        self.__downKeyHandler = None
        self.__upKeyHandler = None

        self.__keys = []
        if bgHref:
            avg.ImageNode(href=bgHref, parent=self)
        for kd in keyDefs:
            if isinstance(kd[0], tuple):
                while len(kd[0]) < self.__codesPerKey:
                    kd[0] += (kd[0][0],)
                key = Key(kd, ovlHref, self.__onCharKeyDown, self.__onCharKeyUp,
                        parent=self)
            else:
                sticky =(self.__stickyShift and 
                        (self.__shiftKeyCode == kd[0] or self.__altGrKeyCode == kd[0])) 
                key = Key(kd, ovlHref, self.__onCommandKeyDown, self.__onCommandKeyUp,
                        sticky=sticky, parent=self)
            self.__keys.append(key)

    @classmethod
    def makeRowKeyDefs(cls, startPos, keySize, spacing, keyStr, shiftKeyStr, 
            altGrKeyStr=None):
        keyDefs = []
        curPos = startPos
        offset = keySize[0]+spacing
        if altGrKeyStr:
            for keyCode, shiftKeyCode, altGrKeyCode in (
                    zip(keyStr, shiftKeyStr, altGrKeyStr)):
                keyDefs.append([(keyCode, shiftKeyCode, altGrKeyCode), curPos, keySize])
                curPos = (curPos[0]+offset, curPos[1])
        else:
            for keyCode, shiftKeyCode in zip(keyStr, shiftKeyStr):
                keyDefs.append([(keyCode, shiftKeyCode), curPos, keySize])
                curPos = (curPos[0]+offset, curPos[1])
        return keyDefs

    def setKeyHandler(self, downHandler, upHandler=None):
        self.__downKeyHandler = downHandler
        self.__upKeyHandler = upHandler

    def reset(self):
        for key in self.__keys:
            key.reset()
        self.__shiftDownCounter = 0
        self.__altGrKeyCounter = 0

    def _getCharKeyCode(self, keyCodes):
        '''
        Return one of a character key's keycodes depending on shift key(s) status.
        Overload this method to change character key keycode handling.
        '''
        if self.__shiftDownCounter:
            return keyCodes[1]
        elif self.__altGrKeyCounter:
            return keyCodes[2]
        else:
            return keyCodes[0]

    def _onCommandKeyDown(self, event, keyCode):
        '''
        Overload this method to add command key functionality.
        '''
        pass

    def _onCommandKeyUp(self, event, keyCode):
        '''
        Overload this method to add command key functionality.
        '''
        pass

    def __onCharKeyDown(self, event, keyCodes):
        if self.__downKeyHandler:
            self.__downKeyHandler(event, self._getCharKeyCode(keyCodes), None)

    def __onCharKeyUp(self, event, keyCodes):
        if self.__upKeyHandler:
            self.__upKeyHandler(event, self._getCharKeyCode(keyCodes), None)

    def __onCommandKeyDown(self, event, keyCode):
        self._onCommandKeyDown(event, keyCode)
        if keyCode == self.__shiftKeyCode:
            self.__shiftDownCounter += 1
        if keyCode == self.__altGrKeyCode:
            self.__altGrKeyCounter += 1
        if self.__downKeyHandler:
            self.__downKeyHandler(event, None, keyCode)

    def __onCommandKeyUp(self, event, keyCode):
        self._onCommandKeyUp(event, keyCode)
        if keyCode == self.__shiftKeyCode:
            if self.__shiftDownCounter > 0:
                self.__shiftDownCounter -= 1
            else:
                g_logger.trace(g_logger.WARNING,
                        'Keyboard: ShiftDownCounter=0 on [%s] up' 
                        %self.__shiftKeyCode)
        elif keyCode == self.__altGrKeyCode:
            if self.__altGrKeyCounter > 0:
                self.__altGrKeyCounter -= 1
        if self.__upKeyHandler:
            self.__upKeyHandler(event, None, keyCode)

