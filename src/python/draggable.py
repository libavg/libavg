class Draggable:
    def __init__(self, node):
        print "__init__"
        self.__node = node
    def enable(self):
        print "enable"
        self.__node.setEventHandler(g_avg.CURSORDOWN, g_avg.MOUSE, self.__start)
    def disable(self):
        self.__node.setEventHandler(g_avg.CURSORDOWN, g_avg.MOUSE, None)
        self.__node.setEventHandler(g_avg.CURSORMOTION, g_avg.MOUSE, None)
        self.__node.setEventHandler(g_avg.CURSORUP, g_avg.MOUSE, None)
    def __start(self, event):
        print "__start"
        groupsNode = self.__node.getParent()
        # TODO: Replace this with reorderChild()
        groupsNode.removeChild(groupsNode.indexOf(self.__node))
        groupsNode.appendChild(self.__node)
        self.__node.setEventCapture()
        self.__node.setEventHandler(g_avg.CURSORMOTION, g_avg.MOUSE, self.__move)
        self.__node.setEventHandler(g_avg.CURSORUP, g_avg.MOUSE, self.__stop)
        self.__baseCursorPos = (event.x, event.y)
        self.__startDragPos = (self.__node.x, self.__node.y)
    def __move(self, event):
        print "__move"
        self.__node.x = self.__startDragPos[0]+event.x-self.__baseCursorPos[0]
        self.__node.y = self.__startDragPos[1]+event.y-self.__baseCursorPos[1]
    def __stop(self, event):
        print "__stop"
        self.__move(event)
        self.__node.setEventHandler(g_avg.CURSORMOTION, g_avg.MOUSE, None)
        self.__node.setEventHandler(g_avg.CURSORUP, g_avg.MOUSE, None)
        self.__node.releaseEventCapture()

def init(avg, Player):
    global g_Player
    global g_avg
    g_Player = Player
    g_avg = avg
