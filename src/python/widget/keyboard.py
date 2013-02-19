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

FEEDBACK_ZOOM_FACTOR = 1.0

# XXX Needs to be moved to a more general place 
# (see uilib2 branch: ui/base.py/StretchNodeBase/_bmpFromSrc()).
def _bmpFromSrc(node, src):
    if isinstance(src, basestring):
        if os.path.isabs(src):
            effectiveSrc = src
        else:
            effectiveSrc = node.getParent().getEffectiveMediaDir() + src
        return avg.Bitmap(effectiveSrc)
    elif isinstance(src, avg.Bitmap):
        return src
    elif src is None:
        return None
    else:
        raise RuntimeError("src must be a string or a Bitmap.")

class Key(avg.DivNode):
    # KeyDef is (keyCode, pos, size, isCommand=False)
    def __init__(self, keyDef, downBmp, feedbackBmp, sticky=False, parent=None,
            **kwargs):
        self.__keyCode = keyDef[0]
        if not(isinstance(self.__keyCode, tuple)):
            self.__keyCode = (self.__keyCode,)
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
        if downBmp:
            if player.isPlaying():
                self.__createImages(downBmp, feedbackBmp)
            else:
                player.subscribe(avg.Player.PLAYBACK_START, 
                        lambda: self.__createImages(downBmp, feedbackBmp))

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

    def onOut(self, event):
        if not self.__cursorID == event.cursorid:
            return
        if not(self.__sticky)  or (not self.__stickyIsDown):
            self.__cursorID = None
            self.__image.opacity = 0.0

    def showFeedback(self, show):
        if show:
            self.__feedbackImage.opacity = 0.95
        else:
            self.__feedbackImage.opacity = 0.0

    def __createImages(self, downBmp, feedbackBmp):
        self.__image = avg.ImageNode(parent=self, opacity=0.0)
        self.__createImage(self.__image, downBmp, 1)
 
        self.__feedbackImage = avg.ImageNode(parent=self, opacity=0.0)
        if feedbackBmp and not(self.__isCommand):
            self.__createImage(self.__feedbackImage, feedbackBmp, 2)
            self.__feedbackImage.pos = (-self.size.x/2, -self.size.y/3 - \
                    self.__feedbackImage.size.y)

    def __createImage(self, node, bmp, sizeFactor):
        canvas = player.createCanvas(id="keycanvas", size=self.size*sizeFactor)
        canvasImage = avg.ImageNode(pos=-self.pos*sizeFactor, parent=canvas.getRootNode())
        canvasImage.setBitmap(bmp)
        canvas.render()
        node.setBitmap(canvas.screenshot())
        player.deleteCanvas('keycanvas')


class Keyboard(avg.DivNode):

    DOWN = avg.Publisher.genMessageID()
    UP = avg.Publisher.genMessageID()
    CHAR = avg.Publisher.genMessageID()

    def __init__(self, bgSrc, downSrc, keyDefs, shiftKeyCode, altGrKeyCode=None,
            stickyShift=False, feedbackSrc=None, parent=None, **kwargs):
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
        if bgSrc:
            bgNode = avg.ImageNode(parent=self)
            bgNode.setBitmap(_bmpFromSrc(self, bgSrc))
        for kd in keyDefs:
            downBmp = _bmpFromSrc(self, downSrc)
            feedbackBmp = _bmpFromSrc(self, feedbackSrc)
            if isinstance(kd[0], tuple):
                while len(kd[0]) < self.__codesPerKey:
                    kd[0] += (kd[0][0],)
                key = Key(kd, downBmp, feedbackBmp, parent=self)
            else:
                sticky =(self.__stickyShift and 
                        (self.__shiftKeyCode == kd[0] or self.__altGrKeyCode == kd[0])) 
                key = Key(kd, downBmp, feedbackBmp, sticky=sticky, parent=self)
            self.__keys.append(key)
        self.subscribe(avg.Node.CURSOR_DOWN, self.__onCursorDown)
        self.__curKeys = {}
        self.__feedbackKey = None

        self.publish(Keyboard.DOWN)
        self.publish(Keyboard.UP)
        self.publish(Keyboard.CHAR)

    @classmethod
    def makeRowKeyDefs(cls, startPos, keySize, spacing, keyStr, shiftKeyStr, 
            altGrKeyStr=None):
        keyDefs = []
        curPos = avg.Point2D(startPos)
        offset = keySize[0]+spacing
        if (len(shiftKeyStr) != len(keyStr) or 
                (altGrKeyStr and len(altGrKeyStr) != len(keyStr))):
            raise RuntimeError("makeRowKeyDefs string lengths must be identical.")

        for i in xrange(len(keyStr)):
            if altGrKeyStr:
                codes = (keyStr[i], shiftKeyStr[i], altGrKeyStr[i])
            else:
                codes = (keyStr[i], shiftKeyStr[i])
            keyDefs.append([codes, curPos, avg.Point2D(keySize), False])
            curPos = (curPos[0]+offset, curPos[1])
        return keyDefs

    def reset(self):
        for key in self.__keys:
            key.reset()
        self.__shiftDownCounter = 0
        self.__altGrKeyCounter = 0

    def __onCursorDown(self, event):
        curKey = self.__findKey(event.pos)
        self.__keyDown(curKey, event)
        event.contact.subscribe(avg.Contact.CURSOR_MOTION, self.__onCursorMotion)
        event.contact.subscribe(avg.Contact.CURSOR_UP, self.__onCursorUp)

    def __onCursorMotion(self, event):
        newKey = self.__findKey(event.pos)
        oldKey = self.__curKeys[event.contact]
        if newKey != oldKey:
            if oldKey:
                oldKey.onOut(event)
                self.notifySubscribers(Keyboard.UP, [oldKey.getCode()[0]])
                if oldKey.isCommand():
                    self.__onCommandKeyUp(oldKey)
            self.__keyDown(newKey, event)

    def __onCursorUp(self, event):
        self.__onCursorMotion(event)
        key = self.__curKeys[event.contact]
        if key:
            key.onUp(event)
            self.notifySubscribers(Keyboard.UP, [key.getCode()[0]])
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
        self.__switchFeedbackKey(key)
        self.__curKeys[event.contact] = key
        if key:
            key.onDown(event)
            if key.isCommand():
                self.__onCommandKeyDown(key)
            else:
                self.__onCharKeyDown(key.getCode())

    def __getCharKeyCode(self, keyCodes):
        if self.__shiftDownCounter:
            return keyCodes[1]
        elif self.__altGrKeyCounter:
            return keyCodes[2]
        else:
            return keyCodes[0]

    def __onCharKeyDown(self, keyCodes):
        self.notifySubscribers(Keyboard.DOWN, [keyCodes[0]])

    def __onCharKeyUp(self, keyCodes):
        self.notifySubscribers(Keyboard.CHAR, [self.__getCharKeyCode(keyCodes)])

    def __onCommandKeyDown(self, key):
        keyCode = key.getCode()[0]
        if not(key.isStickyDown()):
            if keyCode == self.__shiftKeyCode:
                self.__shiftDownCounter += 1
            if keyCode == self.__altGrKeyCode:
                self.__altGrKeyCounter += 1
        self.notifySubscribers(Keyboard.DOWN, [keyCode])

    def __onCommandKeyUp(self, key):
        keyCode = key.getCode()[0]
        if not(key.isStickyDown()):
            if keyCode == self.__shiftKeyCode:
                if self.__shiftDownCounter > 0:
                    self.__shiftDownCounter -= 1
                else:
                    avg.logger.warning('Keyboard: ShiftDownCounter=0 on [%s] up' 
                            %self.__shiftKeyCode)
            elif keyCode == self.__altGrKeyCode:
                if self.__altGrKeyCounter > 0:
                    self.__altGrKeyCounter -= 1
