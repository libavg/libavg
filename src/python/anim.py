#!/usr/bin/python
# -*- coding: utf-8 -*-

# TODO:
# - loops
# - Folgen, Gruppen
# - 
 
import time

class Animation:
    def __init__(self, node, attrName, startValue, endValue, duration):
        self.__node = node
        self.__attrName = attrName
        self.__startValue = startValue
        self.__endValue = endValue
        self.__duration = duration
        g_Player.setTimeout(duration, self.__stop)
        self.__interval = g_Player.setInterval(10, self.__step)
        self.__startTime = time.time()
        self.__step()
    def __step(self):
        part = ((time.time()-self.__startTime)/self.__duration)*1000
        curValue = self.__startValue+(self.__endValue-self.__startValue)*part
        setattr(self.__node, self.__attrName, curValue)
    def __stop(self):
        setattr(self.__node, self.__attrName, self.__endValue)
        g_Player.clearInterval(self.__interval)

def fadeOut(node, duration):
    curValue = getattr(node, "opacity")
    Animation(node, "opacity", curValue, 0, duration)

def fadeIn(node, duration, max):
    curValue = getattr(node, "opacity")
    Animation(node, "opacity", curValue, max, duration)

def init(Player):
    global g_Player
    g_Player = Player
