#!/usr/bin/python
# -*- coding: utf-8 -*-

avg = None
g_Player = None

try:
    from . import avg
except ValueError:
    pass


class Draggable:
    def __init__(self, node, onDragStart=None, onDragEnd=None):
        global g_Player
        g_Player = avg.Player.get()
        self.__node = node
        self.__onDragStart = onDragStart
        self.__onDragEnd = onDragEnd
    def enable(self):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, self.__start)
    def disable(self):
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, None)
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE | avg.TOUCH, None)
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE | avg.TOUCH, None)
    def __start(self, event):
        groupsNode = self.__node.getParent()
        groupsNode.reorderChild(groupsNode.indexOf(self.__node), 
                groupsNode.getNumChildren()-1)
        self.__node.setEventCapture()
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, None)
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE | avg.TOUCH, self.__move)
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE | avg.TOUCH, self.__stop)
        self.__baseCursorPos = (event.x, event.y)
        self.__startDragPos = (self.__node.x, self.__node.y)
        if self.__onDragStart:
            self.__onDragStart(event)
    def __move(self, event):
        self.__node.x = self.__startDragPos[0]+event.x-self.__baseCursorPos[0]
        self.__node.y = self.__startDragPos[1]+event.y-self.__baseCursorPos[1]
    def __stop(self, event):
        self.__move(event)
        self.__node.setEventHandler(avg.CURSORDOWN, avg.MOUSE | avg.TOUCH, self.__start)
        self.__node.setEventHandler(avg.CURSORMOTION, avg.MOUSE | avg.TOUCH, None)
        self.__node.setEventHandler(avg.CURSORUP, avg.MOUSE | avg.TOUCH, None)
        self.__node.releaseEventCapture()
        if self.__onDragEnd:
            self.__onDragEnd(event)

def init(g_avg):
    global avg
    avg = g_avg
