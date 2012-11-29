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
    # KeyDef is (keyCode, pos, size, isCommand=False)
    def __init__(self, keyDef, downHref, feedbackHref, sticky=False, parent=None,
            **kwargs):
        self.__keyCode = keyDef[0]
        kwargs['pos'] = avg.Point2D(keyDef[1])
        kwargs['size'] = avg.Point2D(keyDef[2])
        if len(keyDef) == 4:
            self.__isCommand = keyDef[3]
        else:
            self.__isCommand = False
        super(Key, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__sticky = sticky
        self.__stickyIsDown = False
        self.__cursorID = None
        if downHref:
            if player.isPlaying():
                self.__createImages(downHref, feedbackHref)
            else:
                player.subscribe(avg.Player.PLAYBACK_START, 
                        lambda: self.__createImages(downHref, feedbackHref))

    def reset(self):
        if self.__sticky:
            self.__image.opacity = 0.0
            self.__stickyIsDown = False

    def isCommand(self):
        return self.__isCommand

    def getCode(self):
        return self.__keyCode

    def isStickyDown(self):
        return self.__sticky and self.__stickyIsDown

    def onDown(self, event):
        if self.__cursorID:
            return
        self.__cursorID = event.cursorid
        self.__image.opacity = 1.0

    def onUp(self, event):
        if not self.__cursorID == event.cursorid:
            return
        if self.__sticky:
            self.__stickyIsDown = not(self.__stickyIsDown)
            if not self.__stickyIsDown:
                self.__image.opacity = 0.0
        else:
            self.__image.opacity = 0.0
        self.__cursorID = None

    def onOut(self):
        if not(self.__sticky)  or (not self.__stickyIsDown):
            self.__cursorID = None
            self.__image.opacity = 0.0

    def showFeedback(self, show):
        if show:
            self.__feedbackImage.opacity = 0.95
        else:
            self.__feedbackImage.opacity = 0.0

    def __createImages(self, downHref, feedbackHref):
        self.__image = avg.ImageNode(parent=self, opacity=0.0)
        self.__createImage(self.__image, downHref, 1)

        self.__feedbackImage = avg.ImageNode(parent=self, opacity=0.0)
        if feedbackHref and not(self.__isCommand):
            self.__createImage(self.__feedbackImage, feedbackHref, 2)
            self.__feedbackImage.pos = (-self.size.x/2, -self.size.y/3 - \
                    self.__feedbackImage.size.y)

    def __createImage(self, node, href, sizeFactor):
        if os.path.isabs(href):
            effectiveHref = href
        else:
            effectiveHref = self.getParent().getEffectiveMediaDir() + href
        canvas = player.createCanvas(id="keycanvas", size=self.size*sizeFactor)
        avg.ImageNode(href=effectiveHref, pos=-self.pos*sizeFactor, 
                parent=canvas.getRootNode())
        canvas.render()
        node.setBitmap(canvas.screenshot())
        player.deleteCanvas('keycanvas')


class Keyboard(avg.DivNode):

    DOWN = avg.Publisher.genMessageID()
    UP = avg.Publisher.genMessageID()
    CHAR = avg.Publisher.genMessageID()

    def __init__(self, bgHref, downHref, keyDefs, shiftKeyCode, altGrKeyCode=None,
            stickyShift=False, feedbackHref=None, textarea=None, parent=None, **kwargs):
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
        
        self.__keys = []
        if bgHref:
            avg.ImageNode(href=bgHref, parent=self)
        for kd in keyDefs:
            if isinstance(kd[0], tuple):
                while len(kd[0]) < self.__codesPerKey:
                    kd[0] += (kd[0][0],)
                key = Key(kd, downHref, feedbackHref, parent=self)
            else:
                sticky =(self.__stickyShift and 
                        (self.__shiftKeyCode == kd[0] or self.__altGrKeyCode == kd[0])) 
                key = Key(kd, downHref, feedbackHref, sticky=sticky, parent=self)
            self.__keys.append(key)
        if textarea != None:
            self.__textarea = textarea
        self.subscribe(avg.Node.CURSOR_DOWN, self.__onDown)
        self.__curKeys = {}
        self.__feedbackKey = None

        self.publish(Keyboard.DOWN)
        self.publish(Keyboard.UP)
        self.publish(Keyboard.CHAR)

    def __onDown(self, event):
        curKey = self.__findKey(event.pos)
        self.__switchFeedbackKey(curKey)
        self.__curKeys[event.contact] = curKey
        self.__keyDown(curKey, event)
        event.contact.subscribe(avg.Contact.CURSOR_MOTION, self.__onMotion)
        event.contact.subscribe(avg.Contact.CURSOR_UP, self.__onUp)

    def __onMotion(self, event):
        newKey = self.__findKey(event.pos)
        oldKey = self.__curKeys[event.contact]
        if newKey != oldKey:
            if oldKey:
                oldKey.onOut()
                if oldKey.isCommand():
                    self.__onCommandKeyUp(oldKey)
            self.__switchFeedbackKey(newKey)
            self.__curKeys[event.contact] = newKey
            self.__keyDown(newKey, event)

    def __onUp(self, event):
        self.__onMotion(event)
        key = self.__curKeys[event.contact]
        if key:
            key.onUp(event)
            if key.isCommand():
                self.__onCommandKeyUp(key)
            else:
                self.__onCharKeyUp(key.getCode())
        self.__switchFeedbackKey(None)
        del self.__curKeys[event.contact]

    def __findKey(self, pos):
        for key in self.__keys:
            localPos = key.getRelPos(pos)
            if self.__isInside(localPos, key):
                return key
        return None

    def __isInside(self, pos, node):
        return (pos.x >= 0 and pos.y >= 0 and 
                pos.x <= node.size.x and pos.y <= node.size.y)

    def __switchFeedbackKey(self, newKey):
        if self.__feedbackKey:
            self.__feedbackKey.showFeedback(False)
        self.__feedbackKey = newKey
        if self.__feedbackKey:
            self.__feedbackKey.showFeedback(True)

    def __keyDown(self, key, event):
        if key:
            key.onDown(event)
            if key.isCommand():
                self.__onCommandKeyDown(key)
            else:
                self.__onCharKeyDown(key.getCode())

    @classmethod
    def makeRowKeyDefs(cls, startPos, keySize, spacing, keyStr, shiftKeyStr, 
            altGrKeyStr=None):
        keyDefs = []
        curPos = startPos
        offset = keySize[0]+spacing
        if (len(shiftKeyStr) != len(keyStr) or 
                (altGrKeyStr and len(altGrKeyStr) != len(keyStr))):
            raise RuntimeError("makeRowKeyDefs string lengths must be identical.")

        for i in xrange(len(keyStr)):
            if altGrKeyStr:
                codes = (keyStr[i], shiftKeyStr[i], altGrKeyStr[i])
            else:
                codes = (keyStr[i], shiftKeyStr[i])
            keyDefs.append([codes, curPos, keySize, False])
            curPos = (curPos[0]+offset, curPos[1])
        return keyDefs

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

    def __onCharKeyDown(self, keyCodes):
        self.notifySubscribers(Keyboard.DOWN, [self._getCharKeyCode(keyCodes)])

    def __onCharKeyUp(self, keyCodes):
        self.notifySubscribers(Keyboard.CHAR, [self._getCharKeyCode(keyCodes)])

    def __onCommandKeyDown(self, key):
        keyCode = key.getCode()
        if not(key.isStickyDown()):
            if keyCode == self.__shiftKeyCode:
                self.__shiftDownCounter += 1
            if keyCode == self.__altGrKeyCode:
                self.__altGrKeyCounter += 1
        self.notifySubscribers(Keyboard.DOWN, [keyCode])

    def __onCommandKeyUp(self, key):
        keyCode = key.getCode()
        if not(key.isStickyDown()):
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

    def __upHandler(self, keyCode, cmd):
        if keyCode is None:
            return
        self.__textarea.onKeyDown(ord(keyCode))
