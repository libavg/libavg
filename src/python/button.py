avg = None
g_Player = None

try:
    from . import avg
except ValueError:
    pass

class Button:
    def __init__(self, node, clickCallback):
        self.__node = node
        self.__clickCallback = clickCallback
        self.__upNode = node.getChild(0)
        self.__downNode = node.getChild(1)
        self.__overNode = node.getChild(2)
        self.__disabledNode = node.getChild(3)
        self.__setMode(0)
        self.__isClicking = False
        node.width = self.__upNode.width
        node.height = self.__upNode.height
        node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__onDown)
        self.__node.setEventHandler(avg.CURSOROUT, avg.MOUSE, self.__onOut)
        self.__node.setEventHandler(avg.CURSOROVER, avg.MOUSE, self.__onOver)
    def __onDown(self, event):
        self.__node.setEventCapture()
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onUp)
        self.__isClicking = True
        self.__setMode(1)
    def __onUp(self, event):
        if self.__mode == 1 and self.__isClicking:
            self.__setMode(2)
            self.__clickCallback(self)
        self.__isClicking = False
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE, None)
        self.__node.releaseEventCapture()
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
        global g_Player
        g_Player = avg.Player.get()
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

