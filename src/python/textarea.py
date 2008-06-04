# -*- coding: utf-8 -*-

# TODO: scroller

avg = None
g_Player = None
g_TAStorage = []
g_TAShiftPressed = False

try:
    from . import avg
except ValueError:
    pass

class Textarea:
    """
    Textarea is an extended <words> node that reacts to user input
    (mouse/touch for focus, keyboard for text input).
    It sits in a given container matching its dimensions
    """
    CURSOR_CHAR='|'
    BLUR_OPACITY=0.3
    def __init__(self, parent, bgImageFile=None, disableMouseFocus=False):
        """
        @param parent: a div node with defined dimensions
        @param bgImageFile: path and file name (relative to mediadir) of an image
        that is used as a background for Textarea. The image is stretched to extents
        of the instance
        @param disableMouseFocus: boolean, prevents that mouse can set focus for
        this instance
        """
        global g_Player, g_TAStorage
        g_Player = avg.Player.get()
        self.__parent = parent
        
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
        
        g_TAStorage.append(self)
        
        self.__textNode = textNode
        self.setStyle()
        self.setFocus(False)
        
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
        
    def setStyle(self, font='Arial', size=12, color='000000', multiline=True):
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
        self.__isMultiline = multiline

        if multiline:
            self.__textNode.parawidth = int(self.__parent.width)
        else:
            self.__textNode.parawidth = -1

    def setFocus(self, hasFocus):
        """
        Force the focus (or blur) of this Textarea
        @param hasFocus: boolean
        """
        for ob in g_TAStorage:
            ob.__setFocus(False)

        self.__setFocus(hasFocus)

    def getFocus(self):
        """
        Query the focus status for this Textarea
        """
        return self.__hasFocus
    
    
    def __setFocus(self, hasFocus):
        if hasFocus:
            self.__parent.opacity = 1
        else:
            self.__parent.opacity = self.BLUR_OPACITY
        self.__hasFocus = hasFocus

    def __onClick(self, e):
        print "GOT FOCUS"
        self.setFocus(True)
    
    def onKeyDown(self, keycode):
#        print "DOWN: ",keycode, "->", keystring
        if keycode >= 32 and keycode <= 126:
            self.__appendChar(keycode)
        elif keycode == 8:
            self.__removeLastChar(True)
        # NP/FF clears text
        elif keycode == 12:
            self.setText('')

    def __appendChar(self, ch):
        global g_TAShiftPressed
        
        # don't wrap when Textarea is not multiline
        if not self.__hasFocus or \
            (not self.__isMultiline \
            and self.__textNode.width > self.__parent.width - self.__textNode.size/2.0):
            return
            
        # remove the cursor
        self.__removeLastChar()
        
        if g_TAShiftPressed:
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

def cycleFocus():
    """
    Force a focus cycle among instantiated textareas
    """
    elected = 0
    for ob in g_TAStorage:
        if not ob.getFocus():
            elected = elected + 1
        else:
            break
    
    if elected in (len(g_TAStorage), len(g_TAStorage)-1):
        elected = 0
    else:
        elected = elected + 1

    for ob in g_TAStorage:
        ob.setFocus(False)
    
    g_TAStorage[elected].setFocus(True)
    
def keyCharPressed(kchar):
    """
    Use this method to inject a character to active
    (w/ focus) Textarea, convenience method for keyCodePressed()
    @param kchar: string, a single character
    """
    keyCodePressed(ord(kchar[0]))

def keyCodePressed(keycode):
    """
    Shift a character (ASCII-coded) into the active (w/focus)
    Textarea
    @param keycode: int, ASCII representation of the character
    """
    global g_TAStorage, g_TAShiftPressed
    
    # TAB key cycles focus through textareas
    if keycode == 9:
        cycleFocus()
    elif keycode in (301, 303, 304):
        g_TAShiftPressed = True
    
    for ob in g_TAStorage:
        if ob.getFocus():
            ob.onKeyDown(keycode)

def onKeyDown(e):
    keyCodePressed(e.keycode)

def onKeyUp(e):
    global g_TAStorage, g_TAShiftPressed
    
    if e.keycode in (301, 303, 304):
        g_TAShiftPressed = False

def init(g_avg, catchKeyboard=True):
    """
    This method should be called immediately after avg file
    load (Player.loadFile())
    @param g_avg: avg package
    @param catchKeyboard: boolean, if true events from keyboard are catched
    """
    global avg
    avg = g_avg
    
    if catchKeyboard:
        avg.Player.get().getRootNode().setEventHandler(avg.KEYDOWN, avg.NONE, onKeyDown)
        avg.Player.get().getRootNode().setEventHandler(avg.KEYUP, avg.NONE, onKeyUp)
    