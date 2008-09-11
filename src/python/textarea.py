#!/usr/bin/env python
# -*- coding: utf-8 -*-

avg = None
g_Player = None
g_FocusContext = None
g_LastKeyEvent = None
g_activityCallback = None
g_LastKeyRepeated = 0
g_RepeatDelay = 0.2
g_CharDelay = 0.1

import time

KEYCODE_TAB = 9
KEYCODE_SHTAB = 25
KEYCODE_FORMFEED = 12
KEYCODES_BACKSPACE = (8,127)

DEFAULT_CURSOR_PX = 'tacursorpx.png'
CURSOR_PADDING_PCT = 15
CURSOR_WIDTH_PCT = 4
CURSOR_SPACING_PCT = 4
CURSOR_FLASHING_DELAY = 1000
CURSOR_FLASH_AFTER_INACTIVITY = 200

import os.path
import time
try:
    from . import avg
except ValueError:
    pass

class FocusContext:
    """
    This object serves as a grouping element for TextAreas.
    TextArea elements that belong to the same FocusContext cycle
    focus among themselves. There can be several FocusContextes but
    only one at once can be activated ( using the global function
    setActiveFocusContext() )
    """
    def __init__(self):
        self.__elements = []
        self.__isActive = False

    def isActive(self):
        """
        Test if this FocusContext is active
        """
        return self.__isActive
        
    def register(self, taElement):
        """
        Register a floating textarea with this FocusContext
        @param taElement: TextArea, a reference to a TextArea
        """
        self.__elements.append(taElement)

    def getFocused(self):
        """
        Returns a TextArea element that currently has focus within
        this FocusContext
        """
        for ob in self.__elements:
            if ob.hasFocus():
                return ob
        return None

    def keyCharPressed(self, kchar):
        """
        Use this method to inject a character to active
        (w/ focus) TextArea, convenience method for keyUCodePressed()
        @param kchar: string, a single character
        """
        uch = unicode(kchar, 'utf-8')
        self.keyUCodePressed(ord(uch[0]))

    def keyUCodePressed(self, keycode):
        """
        Shift a character (Unicode) into the active (w/focus)
        TextArea
        @param keycode: int, unicode code point of the character
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
        Emulates a backspace character
        """
        self.keyUCodePressed(KEYCODES_BACKSPACE[0])
    
    def clear(self):
        """
        Clears the active textarea, emulating the press of FF character
        """
        self.keyUCodePressed(KEYCODE_FORMFEED)

    def resetFocuses(self):
        """
        Blurs every TextArea registered within this FocusContext
        """
        for ob in self.__elements:
            ob.clearFocus()
        
    def cycleFocus(self, backwards=False):
        """
        Force a focus cycle among instantiated textareas
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
        """
        return self.__elements

    def _switchActive(self, active):
        """
        De/Activates this FocusContext. Active FocusContexts route to the focused
        textarea the keypress stream
        Use textarea.setActiveFocusContext() instead using it directly
        @param active: boolean, set to True to activate, False to deactivate
        """
        if active:
            self.resetFocuses()
            self.cycleFocus()
        else:
            self.resetFocuses()

        self.__isActive = active


class TextArea:
    """
    TextArea is an extended <words> node that reacts to user input
    (mouse/touch for focus, keyboard for text input).
    It sits in a given container matching its dimensions
    """
    def __init__(self, parent, focusContext=None, bgImageFile=None, disableMouseFocus=False,
                 blurOpacity=0.3, border=0, id='', cursorPixFile=None):
        """
        @param parent: a div node with defined dimensions
        @param focusContext: FocusContext object which groups focus for TextArea elements
        @param bgImageFile: path and file name (relative to mediadir) of an image
        that is used as a background for TextArea. The image is stretched to extents
        of the instance
        @param disableMouseFocus: boolean, prevents that mouse can set focus for
        this instance
        @param blurOpacity: opacity that textarea gets when goes to blur state
        @param border: amount of offsetting pixels that words node will have from image extents
        @param id: optional handle to identify the object when dealing with events. ID uniqueness
        is not guaranteed
        @param cursorPixFile: one-pixel graphic file used to render the cursor. The default one is
        a black one
        """
        global g_Player
        g_Player = avg.Player.get()
        self.__parent = parent
        self.__focusContext = focusContext
        self.__blurOpacity = blurOpacity
        self.__border = border
        self.__id = id
        
        if bgImageFile is not None:
            bgNode = g_Player.createNode("image", {})
            bgNode.href = bgImageFile
            bgNode.width = parent.width
            bgNode.height = parent.height
            parent.appendChild(bgNode)
        
        textNode = g_Player.createNode("words", {'rawtextmode':True})
        
        if not disableMouseFocus:
            parent.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onClick)
            parent.setEventHandler(avg.CURSORUP, avg.TOUCH, self.__onClick)
            
        parent.appendChild(textNode)
        
        if focusContext is not None:
            focusContext.register(self)
        
        if cursorPixFile is None:
            crspx = os.path.dirname(__file__)+'/'+DEFAULT_CURSOR_PX
        else:
            crspx = cursorPixFile

        cursorNode = g_Player.createNode('image', {'href':crspx})
        parent.appendChild(cursorNode)
        self.__flashingCursor = False
        
        self.__cursorNode = cursorNode
        self.__textNode = textNode
        self.__charSize = -1
        self.setStyle()
        self.setFocus(False)

        g_Player.setInterval(CURSOR_FLASHING_DELAY, self.__tickFlashCursor)
        
        self.__lastActivity = 0
    
    def getID(self):
        """
        Returns the ID of the textarea (set on the constructor).
        Useful with single-entry callbacks
        """
        return self.__id
        
    def clearText(self):
        """
        Clears the text
        """
        self.setText('')
        
    def setText(self, uString):
        """
        Set the text on the TextArea
        @param uString: an unicode string
        """
        self.__textNode.text = uString
        self.__resetCursorPosition()
        
    def getText(self):
        """
        Get the text stored and displayed on the TextArea
        """
        return self.__textNode.text
        
    def setStyle(self, font='Arial', size=12, alignment='left', variant='Regular',
                color='000000', multiline=True, cursorWidth=None, flashingCursor=False):
        """
        Set some style parameters of the <words> node of the TextArea
        @param font: font face
        @param size: font size in pixels
        @param alignment: one among 'left', 'right', 'center'
        @param variant: font variant (eg: 'bold')
        @param color: RGB hex text color
        @param multiline: boolean, whether TextArea has to wrap (undefinitely) or stop at full width
        @param cursorWidth: int, width of the cursor in pixels
        """
        self.__textNode.font = font
        self.__textNode.size = int(size)
        self.__textNode.alignment = alignment
        self.__textNode.color = color
        self.__textNode.variant = variant
        self.__isMultiline = multiline
        self.__maxLength = -1
        
        if multiline:
            self.__textNode.parawidth = int(self.__parent.width) - self.__border*2
        else:
            self.__textNode.parawidth = -1
            
        self.__textNode.x = self.__border
        self.__textNode.y = self.__border
        if cursorWidth is not None:
            self.__cursorNode.width = cursorWidth
        else:
            w = float(size) * CURSOR_WIDTH_PCT / 100.0
            if w < 1:
                w = 1
            self.__cursorNode.width = w
        self.__flashingCursor = flashingCursor
        if not flashingCursor:
            self.__cursorNode.opacity = 1
            
        self.__resetCursorPosition()

    
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
            self.__cursorNode.opacity = 1
        else:
            self.clearFocus()
            self.__cursorNode.opacity = 0
            
        self.__hasFocus = hasFocus

    def hasFocus(self):
        """
        Query the focus status for this TextArea
        """
        return self.__hasFocus

    def onKeyDown(self, keycode):
        if keycode in KEYCODES_BACKSPACE:
            self.__removeChar()
            self.__updateLastActivity()
            self.__resetCursorPosition()
        # NP/FF clears text
        elif keycode == KEYCODE_FORMFEED:
            self.clearText()
        # avoid shift-tab, return, zero, delete
        elif keycode not in (0,13,25,63272):
            self.__appendChar(keycode)
            self.__updateLastActivity()
            self.__resetCursorPosition()

    def __onClick(self, e):
        if self.__focusContext is not None:
            if self.__focusContext.isActive():
                self.setFocus(True)
        else:
            self.setFocus(True)
    
    def __appendChar(self, keycode):
        slen = len(self.__textNode.text)
        
        if slen > 0:
            lastCharPos = self.__textNode.getGlyphPos(slen-1)
            # don't wrap when TextArea is not multiline
            if (not self.__isMultiline and
                lastCharPos[0] > self.__parent.width - self.__textNode.size - self.__border * 2):
                return
        
            # don't flee from borders in a multiline textarea
            if (self.__isMultiline and
                lastCharPos[1] > self.__parent.height - self.__textNode.size * 2 - self.__border * 2 and
                lastCharPos[0] > self.__parent.width - self.__textNode.size - self.__border * 2):
                return
        
        # if maximum number of char is specified, honour the limit
        if self.__maxLength > -1 and slen > self.__maxLength:
            return
        
        self.__textNode.text = self.__textNode.text + unichr(keycode)
        
    def __removeChar(self):
        self.__textNode.text = self.__textNode.text[0:-1]

    def __resetCursorPosition(self):
        wslen = len(self.__textNode.text)
        
        if wslen == 0:
            lastCharPos = (0,0)
            lastCharExtents = (0,0)
        else:
            lastCharPos = self.__textNode.getGlyphPos(wslen-1)
            lastCharExtents = self.__textNode.getGlyphSize(wslen-1)

        if lastCharExtents[1] > 0:
            self.__cursorNode.height = lastCharExtents[1] * (1 - CURSOR_PADDING_PCT/100.0)
        else:
            self.__cursorNode.height = self.__textNode.size

        self.__cursorNode.x = lastCharPos[0] + lastCharExtents[0] + self.__textNode.size * CURSOR_SPACING_PCT/100.0
        self.__cursorNode.y = lastCharPos[1] + self.__cursorNode.height * CURSOR_PADDING_PCT/200.0
    
    def __updateLastActivity(self):
        self.__lastActivity = time.time()
        
    def __tickFlashCursor(self):
        if (self.__flashingCursor and
            self.__hasFocus and
            time.time() - self.__lastActivity > CURSOR_FLASH_AFTER_INACTIVITY/1000.0):
            if self.__cursorNode.opacity == 0:
                self.__cursorNode.opacity = 1
            else:
                self.__cursorNode.opacity = 0
        elif self.__hasFocus:
            self.__cursorNode.opacity = 1


##################################
# GLOBAL FUNCTIONS

def onKeyDown(e):
    global g_LastKeyEvent, g_LastKeyRepeated, g_RepeatDelay, g_activityCallback
    
    if e.unicode == 0:
        return
    
    g_LastKeyEvent = e
    g_LastKeyRepeated = time.time() + g_RepeatDelay

    if g_FocusContext is not None:
        g_FocusContext.keyUCodePressed(e.unicode)

        if g_activityCallback is not None:
            g_activityCallback(g_FocusContext)

def onKeyUp(e):
    global g_LastKeyEvent
    
    g_LastKeyEvent = None

def setActiveFocusContext(focusContext):
    """
    @param focusContext: sets the active focusContext. If initialization has been
        made with 'catchKeyboard' == True, the new active focusContext will receive
        the flow of events from keyboard.
    """
    global g_FocusContext
    
    if g_FocusContext is not None:
        g_FocusContext._switchActive(False)
        
    g_FocusContext = focusContext
    g_FocusContext._switchActive(True)

def onFrame():
    global g_LastKeyEvent, g_LastKeyRepeated, g_CharDelay
    if (g_LastKeyEvent is not None and
        time.time() - g_LastKeyRepeated > g_CharDelay and
        g_FocusContext is not None):
        g_FocusContext.keyUCodePressed(g_LastKeyEvent.unicode)
        g_LastKeyRepeated = time.time()

def setActivityCallback(pyfunc):
    """
    If a callback of user interaction is needed (eg: resetting idle timeout)
    just pass a function to this method, which is going to be called at each
    user intervention (keydown, keyup).
    Active focusContext will be passed as argument
    """
    global g_activityCallback
    g_activityCallback = pyfunc
    
def init(g_avg, catchKeyboard=True, repeatDelay=0.2, charDelay=0.1):
    """
    This method should be called immediately after avg file
    load (Player.loadFile())
    @param g_avg: avg package
    @param catchKeyboard: boolean, if true events from keyboard are catched
    """
    global avg, g_RepeatDelay, g_CharDelay
    avg = g_avg
    g_RepeatDelay = repeatDelay
    g_CharDelay = charDelay
    
    avg.Player.get().setOnFrameHandler(onFrame)
    
    if catchKeyboard:
        avg.Player.get().getRootNode().setEventHandler(avg.KEYDOWN, avg.NONE, onKeyDown)
        avg.Player.get().getRootNode().setEventHandler(avg.KEYUP, avg.NONE, onKeyUp)
    
