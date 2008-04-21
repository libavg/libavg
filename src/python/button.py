avg = None
g_Player = None

try:
    from . import avg
except ValueError:
    pass

class Button:
    def __init__(self, node, clickCallback):
        global g_Player
        g_Player = avg.Player.get()
        self.__node = node
        self.__clickCallback = clickCallback
        self.__upNode = node.getChild(0)
        self.__downNode = node.getChild(1)
        self.__overNode = node.getChild(2)
        self.__disabledNode = node.getChild(3)
        node.width = self.__upNode.width
        node.height = self.__upNode.height
        if self.__isMouseOver():
            self.__setMode(2)
        else:
            self.__setMode(0)
        self.__isClicking = False
        node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__onDown)
        self.__node.setEventHandler(avg.CURSOROUT, avg.MOUSE, self.__onOut)
        self.__node.setEventHandler(avg.CURSOROVER, avg.MOUSE, self.__onOver)
    def __isMouseOver(self):
        Event = g_Player.getMouseState()
        relPos = self.__node.getRelPos((Event.x, Event.y))
        return (relPos[0] > 0 and relPos[0] < self.__node.width and
                relPos[1] > 0 and relPos[1] < self.__node.height)
    def __onDown(self, event):
        self.__node.setEventCapture()
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onUp)
        self.__isClicking = True
        self.__setMode(1)
    def __onUp(self, event):
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, None)
        self.__node.releaseEventCapture()
        if self.__mode == 1 and self.__isClicking:
            self.__setMode(2)
            self.__clickCallback(self)
        self.__isClicking = False
    def __onOver(self, event):
        if self.__isClicking:
            self.__setMode(1)
        else:
            self.__setMode(2)
    def __onOut(self, event):
        self.__setMode(0)
    def __setMode(self, newMode):
        self.__mode = newMode
        for i in range(4):
            childNode = self.__node.getChild(i)
            if i == newMode:
                childNode.opacity = 1
            else:
                childNode.opacity = 0

class Checkbox(Button):
    def __init__(self, node, clickCallback=None):
        self.__node = node
        self.__setChecked(False)
        self.__clickCallback = clickCallback
        Button.__init__(self, node, self.__onClick)
    def getState(self):
        return self.__isChecked
    def setState(self, checked):
        self.__setChecked(checked)
    def __setChecked(self, checked):
        self.__isChecked = checked
        if checked:
            self.__node.getChild(4).opacity = 1
        else:
            self.__node.getChild(4).opacity = 0
    def __onClick(self, Event):
        self.__setChecked(not(self.__isChecked))
        if self.__clickCallback != None:
            self.__clickCallback(self)

def init(g_avg):
    global avg
    avg = g_avg

