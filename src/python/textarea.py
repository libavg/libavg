# -*- coding: utf-8 -*-

# TODO: scroller

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
KEYCODE_FORMFEED = 12
KEYCODE_AMPERSAND = 38
KEYCODE_BACKSPACE = 127

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

    def switchActive(self, active):
        if active:
            self.cycleFocus()
        else:
            self.resetFocuses()
        
        self.__isActive = active
        
    def isActive(self):
        return self.__isActive
        
    def register(self, el):
        self.__elements.append(el)

    def getFocused(self):
        """
        Returns a TextArea element that currently has focus within
        this FocusContext
        """
        for ob in self.__elements:
            if ob.getFocus():
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
        @param keycode: int, unicode index of the character
        """
        # TAB key cycles focus through textareas
        if keycode == KEYCODE_TAB:
            self.cycleFocus()
            return

        for ob in self.__elements:
            if ob.getFocus():
                ob.onKeyDown(keycode)

    def backspace(self):
        """
        Emulates a backspace character
        """
        self.keyCodePressed(KEYCODE_BACKSPACE)
    
    def clear(self):
        """
        Clears the entire textarea, emulating the press of FF character
        """
        self.keyCodePressed(KEYCODE_FORMFEED)

    def resetFocuses(self):
        for ob in self.__elements:
            ob.clearFocus()
        
    def cycleFocus(self):
        """
        Force a focus cycle among instantiated textareas
        """
        
        if len(self.__elements) == 0:
            return
        
        elected = 0
        for ob in self.__elements:
            if not ob.getFocus():
                elected = elected + 1
            else:
                break

        if elected in (len(self.__elements), len(self.__elements)-1):
            elected = 0
        else:
            elected = elected + 1

        for ob in self.__elements:
            ob.setFocus(False)

        self.__elements[elected].setFocus(True)
    
    def getRegistered(self):
        return self.__elements
        
class TextArea:
    """
    TextArea is an extended <words> node that reacts to user input
    (mouse/touch for focus, keyboard for text input).
    It sits in a given container matching its dimensions
    """
    def __init__(self, parent, focusContext=None, bgImageFile=None, disableMouseFocus=False,
                 cursorChar='|', blurOpacity=0.3, border=0, id=''):
        """
        @param parent: a div node with defined dimensions
        @param focusContext: FocusContext object which groups focus for TextArea elements
        @param bgImageFile: path and file name (relative to mediadir) of an image
        that is used as a background for TextArea. The image is stretched to extents
        of the instance
        @param disableMouseFocus: boolean, prevents that mouse can set focus for
        this instance
        @param cursorChar: character that should be used as a cursor
        @param blurOpacity: opacity that textarea gets when goes to blur state
        @param border: amount of offsetting pixels that words node will have from image extents
        @param id: optional handle to identify the object when dealing with events. ID uniqueness
        is not guaranteed
        """
        global g_Player
        g_Player = avg.Player.get()
        self.__parent = parent
        self.__focusContext = focusContext
        self.__cursorChar = cursorChar
        self.__blurOpacity = blurOpacity
        self.__border = border
        self.__id = id
        self.__unicodeSequence = []
        
        if bgImageFile is not None:
            bgNode = g_Player.createNode("image", {})
            bgNode.href = bgImageFile
            bgNode.width = parent.width
            bgNode.height = parent.height
            parent.appendChild(bgNode)
        
        textNode = g_Player.createNode("words", {})
        textNode.text = self.__cursorChar
        
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
    
    def getID(self):
        return self.__id
        
    def clearText(self):
        """
        Clears the text
        """
        self.setText('')
        
    def setText(self, text):
        """
        Set the text on the TextArea
        @param text: an utf-8 encoded string
        """
        self.__textNode.text = text + self.__cursorChar

        self.__unicodeSequence = []
        utext = unicode(text, 'utf-8')
        for i in range(len(utext)):
            self.__unicodeSequence.append(ord(utext[i]))
        
    def getText(self):
        """
        Get the text stored and displayed on the TextArea
        """
        return self.__textNode.text[0:-1]
        
    def setStyle(self, font='Arial', size=12, alignment='left', variant='Regular', color='000000', multiline=True):
        """
        Set some style parameters of the <words> node of the TextArea
        @param font: font face
        @param size: font size in pixels
        @param alignment: one among 'left', 'right', 'center'
        @param variant: font variant (eg: 'bold')
        @param color: RGB hex text color
        @param multiline: boolean, whether TextArea has to wrap (undefinitely) or stop at full width
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
        else:
            self.clearFocus()
            
        self.__hasFocus = hasFocus

    def getFocus(self):
        """
        Query the focus status for this TextArea
        """
        return self.__hasFocus

    def onKeyDown(self, keycode):
        if keycode == KEYCODE_BACKSPACE:
            self.__removeLastChar(True)
        # NP/FF clears text
        elif keycode == KEYCODE_FORMFEED:
            self.clearText()
        # avoid shift-tab, return, zero, delete
        elif keycode not in (0,13,25,63272):
            self.__appendChar(keycode)

    def __onClick(self, e):
        if self.__focusContext is not None:
            if self.__focusContext.isActive():
                self.setFocus(True)
        else:
            self.setFocus(True)
    
    def __appendChar(self, keycode):
        lastCharPos = self.__textNode.getGlyphPos(len(self.__textNode.text)-1)
        # don't wrap when TextArea is not multiline
        if (not self.__isMultiline and
            lastCharPos[0] > self.__parent.width - self.__textNode.size - self.__border * 2):
            return
        
        # don't flee from borders in a multiline textarea
        if (self.__isMultiline and
            lastCharPos[1] > self.__parent.height - self.__textNode.size * 2 - self.__border * 2 and
            lastCharPos[0] > self.__parent.width - self.__textNode.size - self.__border * 2):
            return
        
        # if maximum number of char is specified check it
        if self.__maxLength > -1 and len(self.__textNode.text) > self.__maxLength:
            return
                        
        # remove the cursor
        self.__removeLastChar()
        
        # Treat ampersand character (&) with respect, escaping it
        if keycode == KEYCODE_AMPERSAND:
            self.__textNode.text = self.__textNode.text + '&amp;' + self.__cursorChar
        else:
            self.__textNode.text = self.__textNode.text + unichr(keycode).encode('utf-8') + self.__cursorChar
            
        self.__unicodeSequence.append(keycode)
        
    def __removeLastChar(self, delete=False):
        if delete:
            if len(self.__unicodeSequence) == 0:
                return
                
            # verify the byte length of the last inserted character:
            lastChar = self.__unicodeSequence.pop()
            if lastChar == KEYCODE_AMPERSAND:
                until = -6
            elif lastChar < 0x80:
                until = -2
            elif lastChar < 0x800:
                until = -3
            elif lastChar < 0x10000:
                until = -4
            else:
                until = -5
        else:
            until = -1
        
        self.__textNode.text = self.__textNode.text[0:until]
        
        if delete:
            self.__textNode.text = self.__textNode.text + self.__cursorChar


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
        g_FocusContext.switchActive(False)
        
    g_FocusContext = focusContext
    g_FocusContext.switchActive(True)

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
    
