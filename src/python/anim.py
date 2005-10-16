#!/usr/bin/python
# -*- coding: utf-8 -*-

# TODO:
# - loops
# - Folgen, Gruppen
# - Abort animation
# - splines
# - onEnd callback

import time

class SimpleAnim:
    def __init__(self, node, attrName, duration):
        self.node = node
        self.attrName = attrName
        self.duration = duration
        self.startTime = time.time()

class LinearAnim(SimpleAnim):
    def __init__(self, node, attrName, duration, startValue, endValue):
        SimpleAnim.__init__(self, node, attrName, duration)
        g_Player.setTimeout(duration, self.__stop)
        self.interval = g_Player.setInterval(10, self.__step)
        self.__startValue = startValue
        self.__endValue = endValue
        self.__step()
    def __step(self):
        part = ((time.time()-self.startTime)/self.duration)*1000
        curValue = self.__startValue+(self.__endValue-self.__startValue)*part
        setattr(self.node, self.attrName, curValue)
    def __stop(self):
        setattr(self.node, self.attrName, self.__endValue)
        g_Player.clearInterval(self.interval)

class SplineAnim(SimpleAnim):
    def __init__(self, node, attrName, duration, 
            startValue, startSpeed, endValue, endSpeed):
        SimpleAnim.__init__(self, node, attrName, duration)
        g_Player.setTimeout(duration, self.__stop)
        self.interval = g_Player.setInterval(10, self.__step)
        self.__startValue = startValue+0.0
        self.__startSpeed = startSpeed
        self.__endValue = endValue
        self.__endSpeed = endSpeed
        self.__a = -2*(self.__endValue-self.__startValue)+self.__startSpeed+self.__endSpeed
        self.__b = 3*(self.__endValue-self.__startValue)-2*self.__startSpeed-self.__endSpeed
        self.__c = self.__startSpeed
        self.__d = self.__startValue

        self.__step()
    def __step(self):
        part = ((time.time()-self.startTime)/self.duration)*1000
        curValue = ((self.__a*part+self.__b)*part+self.__c)*part+self.__d
        setattr(self.node, self.attrName, curValue)
    def __stop(self):
        setattr(self.node, self.attrName, self.__endValue)
        g_Player.clearInterval(self.interval)
    

def fadeOut(node, duration):
    curValue = getattr(node, "opacity")
    LinearAnim(node, "opacity", duration, curValue, 0)

def fadeIn(node, duration, max):
    curValue = getattr(node, "opacity")
    LinearAnim(node, "opacity", duration, curValue, max)

def init(Player):
    global g_Player
    g_Player = Player
