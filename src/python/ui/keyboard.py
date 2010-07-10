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

from libavg import avg

g_player = avg.Player.get()
g_logger = avg.Logger.get()


class Key(avg.ImageNode):
    def __init__(self, keyDef, ovlHref, onDownCallback, onUpCallback, *args, **kwargs):
        kwargs['pos'] = keyDef[1]
        kwargs['size'] = keyDef[2]
        kwargs['opacity'] = 0.0
        super(Key, self).__init__(*args, **kwargs)

        if not ovlHref is None:
            self.__createImage(ovlHref)
        self.__keyCode = keyDef[0]
        self.__onDownCallback = onDownCallback
        self.__onUpCallback = onUpCallback
        self.__cursorID = None
        self.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, self.__onDown)
        self.setEventHandler(avg.CURSORUP, avg.MOUSE | avg.TOUCH, self.__onUpOut)
        self.setEventHandler(avg.CURSOROUT, avg.MOUSE | avg.TOUCH, self.__onUpOut)

    def __createImage(self, ovlHref):
        canvas = g_player.loadCanvasString(
        '''
            <canvas id="offscreen" size="%s">
                <image href="%s" pos="%s"/>
            </canvas>
        '''
        %(str(self.size),
          str(self.getParent().getEffectiveMediaDir()) + str(ovlHref),
          str(-self.pos)))
        canvas.render()
        self.setBitmap(canvas.screenshot())
        g_player.deleteCanvas('offscreen')

    def __onDown(self, event):
        if not self.__cursorID is None:
            return
        self.__cursorID = event.cursorid

        self.opacity = 1.0
        if not self.__onDownCallback is None:
            self.__onDownCallback(event, self.__keyCode)

    def __onUpOut(self, event):
        if not self.__cursorID == event.cursorid:
            return
        self.__cursorID = None

        self.opacity = 0.0
        if not self.__onUpCallback is None:
            self.__onUpCallback(event, self.__keyCode)


class Keyboard(avg.DivNode):
    '''
    Image based Onscreen Keyboard

    Keyboard uses an overlay image and a list of key definitions to create its' keys.
    It supports character/command keys and shift key functionality.
    '''

    def __init__(self, bgHref, ovlHref, keyDefs, shiftKeyCode, *args, **kwargs):
        '''
        @param  bgHref: background image filename or None
        @param  ovlHref: overlay image filename or None
        @param  keyDefs: list of key definitions
            char key def format: [(<keycode>, <shift keycode>), <pos>, <size>]
            (<shift keycode> is optional)
            cmd key def format: [<keycode>, <pos>, <size>]
        @param  shiftKeyCode: one of the cmd keycodes or None
        '''
        super(Keyboard, self).__init__(*args, **kwargs)

        self.__shiftKeyCode = shiftKeyCode
        self.__shiftDownCounter = 0
        self.__downKeyHandler = None
        self.__upKeyHandler = None

        if not bgHref is None:
            avg.ImageNode(href=bgHref, parent=self)
        for kd in keyDefs:
            if isinstance(kd[0], tuple):
                if not self.__shiftKeyCode is None and len(kd[0]) == 1:
                    kd[0] += kd[0]
                if (self.__shiftKeyCode is None and len(kd[0]) > 0) or \
                        (not self.__shiftKeyCode is None and len(kd[0]) > 1):
                    Key(kd, ovlHref, self.__onCharKeyDown, self.__onCharKeyUp,
                            parent=self)
                else:
                    g_logger.trace(g_logger.ERROR,
                            'Keyboard: Missing keycode(s) for character key: %s' %str(kd))
            else:
                Key(kd, ovlHref, self.__onCommandKeyDown, self.__onCommandKeyUp,
                        parent=self)

    @classmethod
    def makeRowKeyDefs(cls, startPos, keySize, spacing, keyStr, shiftKeyStr):
        '''
        Creates key definitions for a row of uniform keys. Useful for creating the 
        keyDefs parameter of the Keyboard constructor.

        @param startPos: top left pos of the row.
        @param keySize: Size of each key.
        @param spacing: Number of empty pixels between two keys.
        @param keyStr: Unicode string containing the unshifted keycodes
            (i.e. u"qwertzuiopżś")
        @param shiftKeyStr: Unicode string containing the shifted keycodes
            (i.e. u"QWERTZUIOPńć")
        '''
        keyDefs = []
        curPos = startPos
        offset = keySize[0]+spacing
        for keyCode, shiftKeyCode in zip(keyStr, shiftKeyStr):
            keyDefs.append([(keyCode, shiftKeyCode), curPos, keySize])
            curPos = (curPos[0]+offset, curPos[1])
        return keyDefs

    def setKeyHandler(self, downHandler, upHandler=None):
        '''
        Set down and up key handlers.

        Handler paramter list: (event, char, cmd)
        @param  downHandler: handler method to call on key down event or None
        @param  upHandler: handler method to call on key up event or None
        '''
        self.__downKeyHandler = downHandler
        self.__upKeyHandler = upHandler

    def _getCharKeyCode(self, keyCodes):
        '''
        Return one of a character keys' keycodes depending on shift key(s) status.
        Overload this method to change character key keycode handling.
        '''
        return keyCodes[1] if self.__shiftDownCounter else keyCodes[0]

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
        if not self.__downKeyHandler is None:
            self.__downKeyHandler(event, self._getCharKeyCode(keyCodes), None)

    def __onCharKeyUp(self, event, keyCodes):
        if not self.__upKeyHandler is None:
            self.__upKeyHandler(event, self._getCharKeyCode(keyCodes), None)

    def __onCommandKeyDown(self, event, keyCode):
        self._onCommandKeyDown(event, keyCode)
        if keyCode == self.__shiftKeyCode:
            self.__shiftDownCounter += 1
        if not self.__downKeyHandler is None:
            self.__downKeyHandler(event, None, keyCode)

    def __onCommandKeyUp(self, event, keyCode):
        self._onCommandKeyUp(event, keyCode)
        if keyCode == self.__shiftKeyCode:
            if self.__shiftDownCounter > 0:
                self.__shiftDownCounter -= 1
            else:
                g_logger.trace(g_logger.WARNING,
                        'Keyboard: ShiftDownCounter=0 on [%s] up' %self.__shiftKeyCode)
        if not self.__upKeyHandler is None:
            self.__upKeyHandler(event, None, keyCode)

