# TODO:
# - loops
# - Folgen, Gruppen

class SimpleAnim:
    def __init__(self, node, attrName, duration, useInt, onStop):
        self.node = node
        self.attrName = attrName
        self.duration = duration
        self.startTime = g_Player.getFrameTime()
        self.onStop = onStop
        self.useInt = useInt

class LinearAnim(SimpleAnim):
    def __init__(self, node, attrName, duration, startValue, endValue, useInt, onStop):
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop)
        self.__stopTimeout = g_Player.setTimeout(duration, self.__stop)
        self.__interval = g_Player.setInterval(1, self.__step)
        self.__startValue = startValue
        self.__endValue = endValue
        self.__done = False
        self.__step()
    def __step(self):
        if not(self.__done):
            part = ((float(g_Player.getFrameTime())-self.startTime)/self.duration)
            if part > 1.0:
                part = 1.0
            curValue = self.__startValue+(self.__endValue-self.__startValue)*part
            if self.useInt:
                curValue = int(curValue+0.5)
            setattr(self.node, self.attrName, curValue)
    def __stop(self):
        setattr(self.node, self.attrName, self.__endValue)
        self.__done = True
        g_Player.clearInterval(self.__interval)
        if self.onStop != None:
            self.onStop()
    def abort(self):
        if not(self.__done):
            self.__done = True 
            g_Player.clearInterval(self.__interval)
            g_Player.clearInterval(self.__stopTimeout)
    def isDone(self):
        return self.__done

class SplineAnim(SimpleAnim):
    def __init__(self, node, attrName, duration, 
            startValue, startSpeed, endValue, endSpeed, useInt, onStop):
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop)
        g_Player.setTimeout(duration, self.__stop)
        self.interval = g_Player.setInterval(1, self.__step)
        self.__startValue = startValue+0.0
        self.__startSpeed = startSpeed
        self.__endValue = endValue
        self.__endSpeed = endSpeed
        self.__a = -2*(self.__endValue-self.__startValue)+self.__startSpeed+self.__endSpeed
        self.__b = 3*(self.__endValue-self.__startValue)-2*self.__startSpeed-self.__endSpeed
        self.__c = self.__startSpeed
        self.__d = self.__startValue
        self.__done = 0
        self.__step()
    def __step(self):
        if not(self.__done):
            part = ((float(g_Player.getFrameTime())-self.startTime)/self.duration)
            if part > 1.0:
                part = 1.0
            curValue = ((self.__a*part+self.__b)*part+self.__c)*part+self.__d
            if self.useInt:
                curValue = int(curValue+0.5)
            setattr(self.node, self.attrName, curValue)
    def __stop(self):
        setattr(self.node, self.attrName, self.__endValue)
        self.__done = 1
        g_Player.clearInterval(self.interval)
        if self.onStop != None:
            self.onStop()

def fadeOut(node, duration):
    curValue = getattr(node, "opacity")
    LinearAnim(node, "opacity", duration, curValue, 0, 0, None)

def fadeIn(node, duration, max):
    curValue = getattr(node, "opacity")
    LinearAnim(node, "opacity", duration, curValue, max, 0, None)

def init(Player):
    global g_Player
    g_Player = Player
