#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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
# Original author of this module: Thomas Schott <scotty at c-base dot org>
#

import os.path

from libavg import avg, player

g_Logger = avg.Logger.get()

FEEDBACK_ZOOM_FACTOR = 1.0

class Key(avg.DivNode):
    def __init__(self, keyDef, ovlHref, selHref, onDownCallback, onUpCallback,
            onOutCallback=lambda event, keyCode:None, sticky=False, parent=None,
            **kwargs):
        kwargs['pos'] = keyDef[3]
        kwargs['size'] = keyDef[4]
        super(Key, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__image = avg.ImageNode(parent=self, opacity=0.0)
        self.__feedback = keyDef[1]
        if ovlHref:
            self.__createImage(ovlHref, selHref)
        self.__keyCode = keyDef[0]
        self.__onDownCallback = onDownCallback
        self.__onUpCallback = onUpCallback
        self.__onOutCallback = onOutCallback
        self.__sticky = sticky
        self.__repeate = keyDef[2]
        if self.__sticky:
            self.__stickyIsDown = False
        self.__cursorID = None

    def reset(self):
        if self.__sticky:
            self.__image.opacity = 0.0
            self.__stickyIsDown = False

    def __createImage(self, ovlHref, selHref):
        if os.path.isabs(ovlHref):
            effectiveOvlHref = ovlHref
        else:
            effectiveOvlHref = self.getParent().getEffectiveMediaDir() + ovlHref
        canvasOvl = player.loadCanvasString(
        '''
            <canvas id="offscreenOvl" size="%s">
                <image href="%s" pos="%s"/>
            </canvas>
        '''
        %(str(self.size), 
          effectiveOvlHref,
          str(-self.pos)))

        canvasOvl.render()
        self.__image.setBitmap(canvasOvl.screenshot())
        self.__feedbackImage = avg.ImageNode(opacity=0.0)
        if selHref and self.__feedback:
            if os.path.isabs(selHref):
                effectiveSelHref = selHref
            else:
                effectiveSelHref = self.getParent().getEffectiveMediaDir() + selHref
            canvasSel = player.loadCanvasString(
            '''
                <canvas id="offscreenSel" size="%s">
                    <image href="%s" pos="%s"/>
                </canvas>
            '''
            %(str(self.size * 2), 
              effectiveSelHref,
              str(-self.pos * 2)))

            canvasSel.render()
            self.__feedbackImage.setBitmap(canvasSel.screenshot())
            self.__feedbackImage.pos = (-self.size.x/2, -self.size.y/3 - \
                    self.__feedbackImage.size.y)
            self.appendChild(self.__feedbackImage)
            player.deleteCanvas('offscreenSel')
        else:
            self.__feedback = False
        player.deleteCanvas('offscreenOvl')

    def onDown(self, event):
        self.__feedbackImage.opacity = 0.95
        if self.__cursorID:
            return
        self.__pseudoDown(event)
        if self.__repeate:
            self.__repeateID = 0
            self.__repeateTimerID = player.setTimeout(500,
                    lambda event=event: self.__pseudoRepeate(event))

    def onUp(self, event):
        self.__feedbackImage.opacity = 0.0
        if not self.__cursorID == event.cursorid:
            return
        if self.__sticky:
            self.__stickyIsDown = not(self.__stickyIsDown)
            if not self.__stickyIsDown:
                self.__pseudoUp(event)
        else:
            self.__pseudoUp(event)
            if self.__repeate:
                player.clearInterval(self.__repeateTimerID)
                player.clearInterval(self.__repeateID)

    def onOut(self, event):
        self.__feedbackImage.opacity = 0.0
        if not self.__cursorID == event.cursorid:
            return
        if not(self.__sticky)  or (not self.__stickyIsDown):
            self.__cursorID = None
            self.__image.opacity = 0.0
            self.__onOutCallback(event, self.__keyCode)
            if self.__repeate:
                player.clearInterval(self.__repeateTimerID)
                player.clearInterval(self.__repeateID)

    def __pseudoRepeate(self, event):
        if self.__sticky or (not self.__cursorID == event.cursorid):
            return
        self.__repeateID = player.setInterval(100,
                lambda event=event: self.__onUpCallback(event, self.__keyCode))

    def __pseudoDown(self, event):
        self.__cursorID = event.cursorid

        self.__image.opacity = 1.0
        if self.__onDownCallback:
            self.__onDownCallback(event, self.__keyCode)
       
    def __pseudoUp(self, event):
        self.__cursorID = None

        self.__image.opacity = 0.0
        if self.__onUpCallback:
            self.__onUpCallback(event, self.__keyCode)
        

class Keyboard(avg.DivNode):

    def __init__(self, bgHref, ovlHref, keyDefs, shiftKeyCode, altGrKeyCode=None,
            stickyShift=False, selHref=None, textarea=None, parent=None, **kwargs):
        # TODO: shift and altGr handling have some duplicated code.
        super(Keyboard, self).__init__(**kwargs)
        self.registerInstance(self, parent)

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
                key = Key(kd, ovlHref, selHref, self.__onCharKeyDown, self.__onCharKeyUp,
                        parent=self)
            else:
                sticky =(self.__stickyShift and 
                        (self.__shiftKeyCode == kd[0] or self.__altGrKeyCode == kd[0])) 
                key = Key(kd, ovlHref, selHref, self.__onCommandKeyDown,
                        self.__onCommandKeyUp, self.__onCommandKeyUp, sticky=sticky,
                        parent=self)
            self.__keys.append(key)
        if textarea != None:
            self.__textarea = textarea
            self.setKeyHandler(None, self.__upHandler)
        self.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, self.__onDown)

    def __onDown(self, event):
        self.__selectKey(event)
        event.contact.connectListener(self.__selectKey, self.__selectKey)

    def __selectKey(self, event):
        for i in range(len(self.__keys)):
            pos = self.__keys[i].getRelPos(event.pos)
            if pos.x >= 0 and pos.y >= 0:
                if pos.x <= self.__keys[i].size.x and pos.y <= self.__keys[i].size.y:
                    if event.type == avg.CURSORUP:
                        self.__keys[i].onUp(event)  
                    else:                  
                        self.__keys[i].onDown(event)
                    continue
            self.__keys[i].onOut(event)
        
    @classmethod
    def makeRowKeyDefs(cls, startPos, keySize, spacing, feedbackStr, keyStr, shiftKeyStr, 
            altGrKeyStr=None):
        keyDefs = []
        curPos = startPos
        offset = keySize[0]+spacing
        if altGrKeyStr:
            for feedbackCode, keyCode, shiftKeyCode, altGrKeyCode in (
                    zip(feedbackStr, keyStr, shiftKeyStr, altGrKeyStr)):
                if feedbackCode == 'f':
                    keyDefs.append([(keyCode, shiftKeyCode, altGrKeyCode), False, False,
                            curPos, keySize])
                else:
                    keyDefs.append([(keyCode, shiftKeyCode, altGrKeyCode), True, False,
                            curPos, keySize])
                curPos = (curPos[0]+offset, curPos[1])
        else:
            for feedbackCode, keyCode, shiftKeyCode in \
                    zip(feedbackStr, keyStr, shiftKeyStr):
                if feedbackCode == 'f':
                    keyDefs.append([(keyCode, shiftKeyCode), False, False, curPos,
                            keySize])
                else:
                    keyDefs.append([(keyCode, shiftKeyCode), True, False, curPos,
                            keySize])
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
                g_Logger.trace(g_Logger.WARNING,
                        'Keyboard: ShiftDownCounter=0 on [%s] up' 
                        %self.__shiftKeyCode)
        elif keyCode == self.__altGrKeyCode:
            if self.__altGrKeyCounter > 0:
                self.__altGrKeyCounter -= 1
        if self.__upKeyHandler:
            self.__upKeyHandler(event, None, keyCode)

    def __upHandler(self, event, keyCode, cmd):
        if keyCode is None:
            return
        self.__textarea.onKeyDown(ord(keyCode))
