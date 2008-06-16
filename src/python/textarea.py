# -*- coding: utf-8 -*-

# TODO: scroller

avg = None
g_Player = None
g_ShiftPressed = False
g_FocusContext = None
g_LastKeyEvent = None
g_LastKeyRepeated = 0
g_RepeatDelay = 0.2
g_CharDelay = 0.1

import time

try:
    from . import avg
except ValueError:
    pass

class FocusContext:
    """
    This object serves as a grouping element for Textareas.
    Textarea elements that belong to the same FocusContext cycle
    focus among themselves. There can be several FocusContextes but
    only one at once can be activated ( using the global function
    setActiveFocusContext() )
    """
    def __init__(self):
        self.TAElements = []
        self.__isActive = False

    def switchActive(self, active):
        if active:
            self.cycleFocus()
        else:
            self.resetFocuses()
        
        self.__isActive = active
        
    def isActive(self):
        return self.__isActive
        
    def register(self, el):
        self.TAElements.append(el)

    def getFocused(self):
        """
        Returns a Textarea element that currently has focus within
        this FocusContext
        """
        for ob in self.TAElements:
            if ob.getFocus():
                return ob
        return None

    def keyCharPressed(self, kchar):
        """
        Use this method to inject a character to active
        (w/ focus) Textarea, convenience method for keyCodePressed()
        @param kchar: string, a single character
        """
        self.keyCodePressed(ord(kchar[0]))

    def keyCodePressed(self, keycode):
        """
        Shift a character (ASCII-coded) into the active (w/focus)
        Textarea
        @param keycode: int, ASCII representation of the character
        """
        # TAB key cycles focus through textareas
        if keycode == 9:
            self.cycleFocus()

        for ob in self.TAElements:
            if ob.getFocus():
                ob.onKeyDown(keycode)

    def backspace(self):
        self.keyCodePressed(8)
    
    def clear(self):
        self.keyCodePressed(12)

    def resetFocuses(self):
        for ob in self.TAElements:
            ob.clearFocus()
        
    def cycleFocus(self):
        """
        Force a focus cycle among instantiated textareas
        """
        elected = 0
        for ob in self.TAElements:
            if not ob.getFocus():
                elected = elected + 1
            else:
                break

        if elected in (len(self.TAElements), len(self.TAElements)-1):
            elected = 0
        else:
            elected = elected + 1

        for ob in self.TAElements:
            ob.setFocus(False)

        self.TAElements[elected].setFocus(True)
        
class Textarea:
    """
    Textarea is an extended <words> node that reacts to user input
    (mouse/touch for focus, keyboard for text input).
    It sits in a given container matching its dimensions
    """
    CURSOR_CHAR='|'
    BLUR_OPACITY=0.3
    def __init__(self, parent, focusContext=None, bgImageFile=None, disableMouseFocus=False):
        """
        @param parent: a div node with defined dimensions
        @param focusContext: FocusContext object which groups focus for Textarea elements
        @param bgImageFile: path and file name (relative to mediadir) of an image
        that is used as a background for Textarea. The image is stretched to extents
        of the instance
        @param disableMouseFocus: boolean, prevents that mouse can set focus for
        this instance
        """
        global g_Player
        g_Player = avg.Player.get()
        self.__parent = parent
        self.__focusContext = focusContext
        
        if bgImageFile is not None:
            bgNode = g_Player.createNode("<image/>")
            bgNode.href = bgImageFile
            bgNode.width = parent.width
            bgNode.height = parent.height
            parent.appendChild(bgNode)
        
        textNode = g_Player.createNode("<words/>")
        textNode.text = self.CURSOR_CHAR
        
        if not disableMouseFocus:
            parent.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onClick)
            parent.setEventHandler(avg.CURSORUP, avg.TOUCH, self.__onClick)
            
        parent.appendChild(textNode)
        
        if focusContext is not None:
            focusContext.register(self)
        
        self.__textNode = textNode
        self.__charSize = -1
        self.setStyle()
        self.setFocus(False)
    
    def clearText(self):
        """
        Clears the text
        """
        self.setText('')
        
    def setText(self, text):
        """
        Set the text on the Textarea
        @param text: a string
        """
        self.__textNode.text = text + self.CURSOR_CHAR
        
    def getText(self):
        """
        Get the text stored and displayed on the Textarea
        """
        return self.__textNode.text[0:-1]
        
    def setStyle(self, font='Arial', size=12, weight='normal', color='000000', multiline=True):
        """
        Set some style parameters of the <words> node of the Textarea
        @param font: font face
        @param size: font size in pixels
        @param color: RGB hex text color
        @param multiline: boolean, whether Textarea has to wrap (undefinitely) or stop at full width
        """
        self.__textNode.font = font
        self.__textNode.size = int(size)
        self.__textNode.color = color
        self.__textNode.weight = weight
        self.__isMultiline = multiline
        
        if multiline:
            self.__textNode.parawidth = int(self.__parent.width)
        else:
            self.__textNode.parawidth = -1
    
    def setMaxLength(self, maxlen):
        """
        Set character limit of the input
        @param maxlen: max number of character allowed
        """
        self.__maxLength = maxlen
        
    def clearFocus(self):
        """
        Compact form to blur the Textarea
        """
        self.__parent.opacity = self.BLUR_OPACITY
        self.__hasFocus = False
        
    def setFocus(self, hasFocus):
        """
        Force the focus (or blur) of this Textarea
        @param hasFocus: boolean
        """
        if self.__focusContext is not None:
            self.__focusContext.resetFocuses()
            
        if hasFocus:
            self.__parent.opacity = 1
        else:
            self.clearFocus()
            
        self.__hasFocus = hasFocus

    def getFocus(self):
        """
        Query the focus status for this Textarea
        """
        return self.__hasFocus
    

    def onKeyDown(self, keycode):
#        print "DOWN: ",keycode, "->", keystring
        if keycode >= 32 and keycode <= 126:
            self.__appendChar(keycode)
        elif keycode == 8:
            self.__removeLastChar(True)
        # NP/FF clears text
        elif keycode == 12:
            self.clearText()

    def __onClick(self, e):
        if self.__focusContext is not None:
            if self.__focusContext.isActive():
                self.setFocus(True)
        else:
            self.setFocus(True)
    
    def __appendChar(self, ch):
        global g_ShiftPressed

#        print "LASTCHARX/Y:",self.__textNode.lastcharx, self.__textNode.lastchary
#        print "LIMITS:", self.__parent.height - self.__textNode.size*2, self.__parent.width - self.__textNode.size/1.5
        
        # don't wrap when Textarea is not multiline
        if not self.__isMultiline and \
            self.__textNode.lastcharx > self.__parent.width - self.__textNode.size:
            return
        
        # don't flee from borders in a multiline textarea
        if self.__isMultiline and \
            self.__textNode.lastchary > self.__parent.height - self.__textNode.size*2 and \
            self.__textNode.lastcharx > self.__parent.width - self.__textNode.size:
            return
        
        # if maximum number of char is specified check it
        if self.__maxLength > -1 and len(self.__textNode.text) > self.__maxLength:
            return
                        
        # remove the cursor
        self.__removeLastChar()
        
        if g_ShiftPressed:
            if ch >= 97 and ch <= 122:
                ch = ch - 32
            elif ch >= 48 and ch <= 57:
#                ch = self.CHARMAP[ch]
                ch = ch - 16

        self.__textNode.text = self.__textNode.text + chr(ch) + self.CURSOR_CHAR
        
    def __removeLastChar(self, delete=False):
        if delete:
            until = -2
        else:
            until = -1
        
        self.__textNode.text = self.__textNode.text[0:until]
        
        if delete:
            self.__textNode.text = self.__textNode.text + self.CURSOR_CHAR
            
    
def onKeyDown(e):
    global g_ShiftPressed, g_LastKeyEvent, g_LastKeyRepeated, g_RepeatDelay
    
#    print "KEYDOWN:",e.keycode,e.keystring
    if e.keycode in (301, 303, 304):
        g_ShiftPressed = True
    else:
        g_LastKeyEvent = e
        g_LastKeyRepeated = time.time() + g_RepeatDelay

    if g_FocusContext is not None:
        g_FocusContext.keyCodePressed(e.keycode)

def onKeyUp(e):
    global g_ShiftPressed, g_LastKeyEvent
    
    if e.keycode in (301, 303, 304):
        g_ShiftPressed = False
    else:
        g_LastKeyEvent = None

def setActiveFocusContext(focusContext):
    """
    @param afc: 
    """
    global g_FocusContext
    
    if g_FocusContext is not None:
        g_FocusContext.switchActive(False)
        
    g_FocusContext = focusContext
    g_FocusContext.switchActive(True)

def onFrame():
    global g_LastKeyEvent, g_LastKeyRepeated, g_CharDelay
    if g_LastKeyEvent is not None and \
        time.time() - g_LastKeyRepeated > g_CharDelay and \
        g_FocusContext is not None:
        g_FocusContext.keyCodePressed(g_LastKeyEvent.keycode)
        g_LastKeyRepeated = time.time()
        
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
    