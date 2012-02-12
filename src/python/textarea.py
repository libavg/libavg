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
# Original author of this module is Marco Fagiolini <mfx at archi-me-des dot de>
#

"""
Single/Multi-Line editable text field widget for libavg

textarea module provides two classes:

1. TextArea
    This is the implementation of the widget. Every instantiated TextArea
    represents an editable text field, which can be set up with several styles
    and behaviors.
    
2. FocusContext
    This helps to easily route the events that comes from keyboards to an
    appropriate TextArea instance, cycling focuses and dispatching events on
    the selected field.

"""

g_Player = None
g_FocusContext = None
g_LastKeyEvent = None
g_activityCallback = None
g_LastKeyRepeated = 0
g_RepeatDelay = 0.2
g_CharDelay = 0.1

KEYCODE_TAB = 9
KEYCODE_LINEFEED = 13
KEYCODE_SHTAB = 25
KEYCODE_FORMFEED = 12
KEYCODE_CRS_UP = 63232
KEYCODE_CRS_DOWN = 63233
KEYCODE_CRS_LEFT = 63234
KEYCODE_CRS_RIGHT = 63235
KEYCODES_BACKSPACE = (8,127)
KEYCODES_DEL = 63272

CURSOR_PADDING_PCT = 15
CURSOR_WIDTH_PCT = 4
CURSOR_SPACING_PCT = 4
CURSOR_FLASHING_DELAY = 600
CURSOR_FLASH_AFTER_INACTIVITY = 200

DEFAULT_BLUR_OPACITY = 0.3

import time

from libavg import avg
from avg import Point2D


class FocusContext(object):
    """
    This class helps to group TextArea elements
    
    TextArea elements that belong to the same FocusContext cycle focus among
    themselves. There can be several FocusContextes but only one active at once
    ( using the global function setActiveFocusContext() )
    """
    def __init__(self):
        self.__elements = []
        self.__isActive = False

    def isActive(self):
        """
        Test if this FocusContext is currently active
        """
        return self.__isActive
        
    def register(self, taElement):
        """
        Register a floating textarea on this FocusContext
        
        @param taElement: a TextArea instance
        """
        self.__elements.append(taElement)

    def getFocused(self):
        """
        Query the TextArea element that currently has focus
        
        @return: TextArea instance or None
        """
        for ob in self.__elements:
            if ob.hasFocus():
                return ob
        return None

    def keyCharPressed(self, kchar):
        """
        Inject an utf-8 encoded characted into the flow
        
        Shift a character (Unicode keycode) into the active (w/focus) TextArea
        @type kchar: string
        @param kchar: a single character (if more than one, the following are ignored)
        """
        uch = unicode(kchar, 'utf-8')
        self.keyUCodePressed(ord(uch[0]))

    def keyUCodePressed(self, keycode):
        """
        Inject an Unicode code point into the flow
        
        Shift a character (Unicode keycode) into the active (w/focus) TextArea
        @type keycode: int
        @param keycode: unicode code point of the character
        """
        # TAB key cycles focus through textareas
        if keycode == KEYCODE_TAB:
            self.cycleFocus()
            return
        # Shift-TAB key cycles focus through textareas backwards
        if keycode == KEYCODE_SHTAB:
            self.cycleFocus(True)
            return

        for ob in self.__elements:
            if ob.hasFocus():
                ob.onKeyDown(keycode)

    def backspace(self):
        """
        Emulate a backspace character keypress
        """
        self.keyUCodePressed(KEYCODES_BACKSPACE[0])
    
    def delete(self):
        """
        Emulate a delete character keypress
        """
        self.keyUCodePressed(KEYCODE_DEL)

    def clear(self):
        """
        Clear the active textarea, emulating the press of FF character
        """
        self.keyUCodePressed(KEYCODE_FORMFEED)

    def resetFocuses(self):
        """
        Blur every TextArea registered within this FocusContext
        """
        for ob in self.__elements:
            ob.clearFocus()
        
    def cycleFocus(self, backwards=False):
        """
        Force a focus cycle among instantiated textareas
        
        TAB/Sh-TAB keypress is what is translated in a focus cycle.
        @param backwards: as default, the method cycles following the order
            that has been followed during the registration of TextArea
            instances. Setting this to True, the order is inverted.
        """
        
        els = []
        els.extend(self.__elements)
        
        if len(els) == 0:
            return
        
        if backwards:
            els.reverse()
        
        elected = 0
        for ob in els:
            if not ob.hasFocus():
                elected = elected + 1
            else:
                break
            
        # elects the first if no ta are in focus or if the
        # last one has it
        if elected in (len(els), len(els)-1):
            elected = 0
        else:
            elected = elected + 1

        for ob in els:
            ob.setFocus(False)

        els[elected].setFocus(True)
    
    def getRegistered(self):
        """
        Returns a list of TextArea currently registered within this FocusContext
        @return: a list of registered TextArea instances
        """
        return self.__elements

    def _switchActive(self, active):
        if active:
            self.resetFocuses()
            self.cycleFocus()
        else:
            self.resetFocuses()

        self.__isActive = active


class TextArea(object):
    """
    TextArea class is a libavg widget to create editable text fields
    
    TextArea is an extended <words> node that reacts to user input (mouse/touch for 
    focus, keyboard for text input). Can be set as a single line or span to multiple
    lines.
    It sits in a given container matching its dimensions, therefore the appropriate
    way to create it, is to set a <div> node with defined width/height attributes.
    """
    def __init__(self, parent, focusContext=None, disableMouseFocus=False, id=''):
        """
        @param parent: a div node with defined dimensions
        @param focusContext: FocusContext object which directs focus for TextArea elements
        @param disableMouseFocus: boolean, prevents that mouse can set focus for
            this instance
        @param id: optional handle to identify the object when dealing with events. ID
            uniqueness is not guaranteed
        """
        global g_Player
        g_Player = avg.Player.get()
        self.__parent = parent
        self.__focusContext = focusContext
        self.__blurOpacity = DEFAULT_BLUR_OPACITY
        self.__border = 0
        self.__id = id
        self.__data = []
        self.__cursorPosition = 0
        
        textNode = g_Player.createNode("words", {'rawtextmode':True})
        
        if not disableMouseFocus:
            parent.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onClick)
            parent.setEventHandler(avg.CURSORUP, avg.TOUCH, self.__onClick)
            
        parent.appendChild(textNode)
        
        cursorContainer = g_Player.createNode('div', {})
        cursorNode = g_Player.createNode('line', {'color': '000000'})
        parent.appendChild(cursorContainer)
        cursorContainer.appendChild(cursorNode)
        self.__flashingCursor = False
        
        self.__cursorContainer = cursorContainer
        self.__cursorNode = cursorNode
        self.__textNode = textNode
        self.__charSize = -1
        self.setStyle()

        if focusContext is not None:
            focusContext.register(self)
            self.setFocus(False)
        else:
            self.setFocus(True)

        g_Player.setInterval(CURSOR_FLASHING_DELAY, self.__tickFlashCursor)
        
        self.__lastActivity = 0
    
    def getID(self):
        """
        Returns the ID of the textarea (set on the constructor).
        """
        return self.__id
        
    def clearText(self):
        """
        Clears the text
        """
        self.setText(u'')
        
    def setText(self, uString):
        """
        Set the text on the TextArea
        
        @param uString: an unicode string (or an utf-8 encoded string)
        """
        if not isinstance(uString, unicode):
            uString = unicode(uString, 'utf-8')
            
        self.__data = []
        for c in uString:
            self.__data.append(c)
            
        self.__cursorPosition = len(self.__data)
        self.__update()
        
    def getText(self):
        """
        Get the text stored and displayed on the TextArea
        """
        return self.__getUnicodeFromData()
        
    def setStyle(self, font='Arial', fontsize=12, alignment='left', variant='Regular',
                color='000000', multiline=True, cursorWidth=None, border=0,
                blurOpacity=DEFAULT_BLUR_OPACITY, flashingCursor=False,
                cursorColor='000000', lineSpacing=0, letterSpacing=0):
        """
        Set TextArea's graphical appearance
        @param font: font face
        @param fontsize: font size in pixels
        @param alignment: one among 'left', 'right', 'center'
        @param variant: font variant (eg: 'bold')
        @param color: RGB hex for text color
        @param multiline: boolean, whether TextArea has to wrap (undefinitely)
            or stop at full width
        @param cursorWidth: int, width of the cursor in pixels
        @param border: amount of offsetting pixels that words node will have from image
            extents
        @param blurOpacity: opacity that textarea gets when goes to blur state
        @param flashingCursor: whether the cursor should flash or not
        @param cursorColor: RGB hex for cursor color
        @param lineSpacing: linespacing property of words node
        @param letterSpacing: letterspacing property of words node
        """
        self.__textNode.font = font
        self.__textNode.fontsize = int(fontsize)
        self.__textNode.alignment = alignment
        self.__textNode.color = color
        self.__textNode.variant = variant
        self.__textNode.linespacing = lineSpacing
        self.__textNode.letterspacing = letterSpacing
        self.__isMultiline = multiline
        self.__border = border
        self.__maxLength = -1
        self.__blurOpacity = blurOpacity
        
        if multiline:
            self.__textNode.width = int(self.__parent.width) - self.__border * 2
            self.__textNode.wrapmode = 'wordchar'
        else:
            self.__textNode.width = 0 
            
        self.__textNode.x = self.__border
        self.__textNode.y = self.__border
        
        self.__cursorNode.color = cursorColor
        if cursorWidth is not None:
            self.__cursorNode.strokewidth = cursorWidth
        else:
            w = float(fontsize) * CURSOR_WIDTH_PCT / 100.0
            if w < 1:
                w = 1
            self.__cursorNode.strokewidth = w
        x  = self.__cursorNode.strokewidth / 2.0
        self.__cursorNode.pos1 = Point2D(x, self.__cursorNode.pos1.y)
        self.__cursorNode.pos2 = Point2D(x, self.__cursorNode.pos2.y)
        
        self.__flashingCursor = flashingCursor
        if not flashingCursor:
            self.__cursorContainer.opacity = 1
            
        self.__updateCursor()

    
    def setMaxLength(self, maxlen):
        """
        Set character limit of the input
        
        @param maxlen: max number of character allowed
        """
        self.__maxLength = maxlen
        
    def clearFocus(self):
        """
        Compact form to blur the TextArea
        """
        self.__parent.opacity = self.__blurOpacity
        self.__hasFocus = False
        
    def setFocus(self, hasFocus):
        """
        Force the focus (or blur) of this TextArea
        
        @param hasFocus: boolean
        """
        if self.__focusContext is not None:
            self.__focusContext.resetFocuses()
            
        if hasFocus:
            self.__parent.opacity = 1
            self.__cursorContainer.opacity = 1
        else:
            self.clearFocus()
            self.__cursorContainer.opacity = 0
            
        self.__hasFocus = hasFocus

    def hasFocus(self):
        """
        Query the focus status for this TextArea
        """
        return self.__hasFocus

    def showCursor(self, show):
        if show:
            avg.fadeIn(self.__cursorNode, 200)
        else:
            avg.fadeOut(self.__cursorNode, 200)

    def onKeyDown(self, keycode):
        """
        Inject a keycode into TextArea flow
        
        Used mainly by FocusContext. It can be used directly, but the best option
        is always to use a FocusContext helper, which exposes convenience method for
        injection.
        @param keycode: characted to insert
        @type keycode: int (SDL reference)
        """
        # Ensure that the cursor is shown
        if self.__flashingCursor:
            self.__cursorContainer.opacity = 1
        
        if keycode in KEYCODES_BACKSPACE:
            self.__removeChar(left=True)
            self.__updateLastActivity()
            self.__updateCursor()
        elif keycode == KEYCODES_DEL:
            self.__removeChar(left=False)
            self.__updateLastActivity()
            self.__updateCursor()
        # NP/FF clears text
        elif keycode == KEYCODE_FORMFEED:
            self.clearText()
        elif keycode in (KEYCODE_CRS_UP, KEYCODE_CRS_DOWN, KEYCODE_CRS_LEFT, KEYCODE_CRS_RIGHT):
            if keycode == KEYCODE_CRS_LEFT and self.__cursorPosition > 0:
                self.__cursorPosition -= 1
                self.__update()
            elif keycode == KEYCODE_CRS_RIGHT and self.__cursorPosition < len(self.__data):
                self.__cursorPosition += 1
                self.__update()
            elif keycode == KEYCODE_CRS_UP and self.__cursorPosition != 0:
                self.__cursorPosition = 0
                self.__update()
            elif keycode == KEYCODE_CRS_DOWN and self.__cursorPosition != len(self.__data):
                self.__cursorPosition = len(self.__data)
                self.__update()
        # add linefeed only on multiline textareas
        elif keycode == KEYCODE_LINEFEED and self.__isMultiline:
            self.__appendUChar('\n')
        # avoid shift-tab, return, zero, delete
        elif keycode not in (KEYCODE_LINEFEED, 0, 25, 63272):
            self.__appendKeycode(keycode)
            self.__updateLastActivity()
            self.__updateCursor()

    def __onClick(self, e):
        if self.__focusContext is not None:
            if self.__focusContext.isActive():
                self.setFocus(True)
        else:
            self.setFocus(True)
    
    def __getUnicodeFromData(self):
        return u''.join(self.__data)
    
    def __appendKeycode(self, keycode):
        self.__appendUChar(unichr(keycode))
        
    def __appendUChar(self, uchar):
        # if maximum number of char is specified, honour the limit
        if self.__maxLength > -1 and len(self.__data) > self.__maxLength:
            return
        
        # Boundary control
        if len(self.__data) > 0:
            maxCharDim = self.__textNode.fontsize
            lastCharPos = self.__textNode.getGlyphPos(len(self.__data) - 1)
            if (not self.__isMultiline and
                 lastCharPos[0] + maxCharDim * 1.5 > self.__parent.width - self.__border * 2):
                return
       
            if  (self.__isMultiline and 
                    lastCharPos[1] + maxCharDim * 2 > self.__parent.height - self.__border * 2):
                if (lastCharPos[0] + maxCharDim * 1.5 > self.__parent.width - self.__border * 2):
                    return
                if (ord(uchar) == 10 and
                    lastCharPos[1] + maxCharDim * 2 > self.__parent.height - self.__border * 2):
                    return

        self.__data.insert(self.__cursorPosition, uchar)
        self.__cursorPosition += 1
        self.__update()
        
    def __removeChar(self, left=True):
        if left and self.__cursorPosition > 0:
            self.__cursorPosition -= 1
            del self.__data[self.__cursorPosition]
            self.__update()
        elif not left and self.__cursorPosition < len(self.__data):
            del self.__data[self.__cursorPosition]
            self.__update()

    def __update(self):
        self.__textNode.text = self.__getUnicodeFromData()
        self.__updateCursor()
        
    def __updateCursor(self):
        if self.__cursorPosition == 0:
            lastCharPos = (0,0)
            lastCharExtents = (0,0)
        else:
            lastCharPos = self.__textNode.getGlyphPos(self.__cursorPosition - 1)
            lastCharExtents = self.__textNode.getGlyphSize(self.__cursorPosition - 1)
            
            if self.__data[self.__cursorPosition - 1] == '\n':
                lastCharPos = (0, lastCharPos[1] + lastCharExtents[1])
                lastCharExtents = (0, lastCharExtents[1])
       
        xPos = self.__cursorNode.pos2.x
        if lastCharExtents[1] > 0:
            self.__cursorNode.pos2 = Point2D(xPos, lastCharExtents[1] * (1 - CURSOR_PADDING_PCT/100.0))
        else:
            self.__cursorNode.pos2 = Point2D(xPos, self.__textNode.fontsize)
        
        self.__cursorContainer.x = lastCharPos[0] + lastCharExtents[0] + self.__border
        self.__cursorContainer.y = (lastCharPos[1] +
            self.__cursorNode.pos2.y * CURSOR_PADDING_PCT/200.0 + self.__border)
        
    def __updateLastActivity(self):
        self.__lastActivity = time.time()
        
    def __tickFlashCursor(self):
        if (self.__flashingCursor and
            self.__hasFocus and
            time.time() - self.__lastActivity > CURSOR_FLASH_AFTER_INACTIVITY/1000.0):
            if self.__cursorContainer.opacity == 0:
                self.__cursorContainer.opacity = 1
            else:
                self.__cursorContainer.opacity = 0
        elif self.__hasFocus:
            self.__cursorContainer.opacity = 1


##################################
# MODULE FUNCTIONS

def init(g_avg, catchKeyboard=True, repeatDelay=0.2, charDelay=0.1):
    """
    Initialization routine for the module
    
    This method should be called immediately after avg file
    load (Player.loadFile())
    @param g_avg: avg package
    @param catchKeyboard: boolean, if true events from keyboard are catched
    @param repeatDelay: wait time (seconds) before starting to repeat a key which
        is held down
    @param charDelay: delay among character repetition (of an steadily pressed key)
    """
    global avg, g_RepeatDelay, g_CharDelay
    avg = g_avg
    g_RepeatDelay = repeatDelay
    g_CharDelay = charDelay
    
    avg.Player.get().setOnFrameHandler(_onFrame)
    
    if catchKeyboard:
        avg.Player.get().getRootNode().setEventHandler(avg.KEYDOWN, avg.NONE, _onKeyDown)
        avg.Player.get().getRootNode().setEventHandler(avg.KEYUP, avg.NONE, _onKeyUp)

def setActiveFocusContext(focusContext):
    """
    Tell the module what FocusContext is presently active
    
    Only one FocusContext at once can be set 'active' and therefore
    prepared to receive the flow of user events from keyboard.
    @param focusContext: set the active focusContext. If initialization has been
        made with 'catchKeyboard' == True, the new active focusContext will receive
        the flow of events from keyboard.
    """
    global g_FocusContext
    
    if g_FocusContext is not None:
        g_FocusContext._switchActive(False)
        
    g_FocusContext = focusContext
    g_FocusContext._switchActive(True)

def setActivityCallback(pyfunc):
    """
    Set a callback that is called at every keyboard's keypress
    
    If a callback of user interaction is needed (eg: resetting idle timeout)
    just pass a function to this method, which is going to be called at each
    user intervention (keydown, keyup).
    Active focusContext will be passed as argument
    """
    global g_activityCallback
    g_activityCallback = pyfunc
    

def _onFrame():
    global g_LastKeyEvent, g_LastKeyRepeated, g_CharDelay
    if (g_LastKeyEvent is not None and
        time.time() - g_LastKeyRepeated > g_CharDelay and
        g_FocusContext is not None):
        g_FocusContext.keyUCodePressed(g_LastKeyEvent.unicode)
        g_LastKeyRepeated = time.time()

def _onKeyDown(e):
    global g_LastKeyEvent, g_LastKeyRepeated, g_RepeatDelay, g_activityCallback

    if e.unicode == 0:
        return

    g_LastKeyEvent = e
    g_LastKeyRepeated = time.time() + g_RepeatDelay

    if g_FocusContext is not None:
        g_FocusContext.keyUCodePressed(e.unicode)

        if g_activityCallback is not None:
            g_activityCallback(g_FocusContext)

def _onKeyUp(e):
    global g_LastKeyEvent

    g_LastKeyEvent = None
