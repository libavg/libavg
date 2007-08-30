# TODO:
# - loops
# - Folgen, Gruppen

class SimpleAnim:
    """
    Base class for animations that change libavg node attributes by interpolating
    over a set amount of time. Constructing an animation object starts the
    animation. If abort() isn't needed, there is no need to hold on to the object - 
    it will exist exactly as long as the animation lasts and then disappear.
    """
    def __init__(self, node, attrName, duration, useInt, onStop):
        self.node = node
        self.attrName = attrName
        self.duration = duration
        self.startTime = g_Player.getFrameTime()
        self.onStop = onStop
        self.useInt = useInt

class LinearAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node by interpolating linearly
    between start and end values. 
    """
    def __init__(self, node, attrName, duration, startValue, endValue, useInt, onStop):
        """
        @param node: The libavg node object to animate.
        @param attrName: The name of the attribute to change. Must be a numeric
        attribute.
        @param duration: The length of the animation in milliseconds.
        @param startValue: Initial value of the attribute.
        @param endValue: Value of the attribute after duration has elapsed.
        @param useInt: If True, the attribute is always set to an integer value.
        @param onStop: Python callable to invoke when duration has elapsed and
        the animation has finished. This can be used to chain 
        animations together by using lambda to create a second animation.
        """
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop)
        self.__stopTimeout = g_Player.setTimeout(duration, self.__stop)
        self.__interval = g_Player.setOnFrameHandler(self.__step)
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
        """
        Stops the animation.
        """
        if not(self.__done):
            self.__done = True 
            g_Player.clearInterval(self.__interval)
            g_Player.clearInterval(self.__stopTimeout)
    def isDone(self):
        """
        Returns True if the animation has run its course.
        """
        return self.__done

class SplineAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node by interpolating 
    between start and end values using a cubic spline.
    """
    def __init__(self, node, attrName, duration, 
            startValue, startSpeed, endValue, endSpeed, useInt, onStop):
        """
        @param node: The libavg node object to animate.
        @param attrName: The name of the attribute to change. Must be a numeric
        attribute.
        @param duration: The length of the animation in milliseconds.
        @param startValue: Initial value of the attribute.
        @param startSpeed: Initial speed of the animation.
        @param endValue: Value of the attribute after duration has elapsed.
        @param endSpeed: Final speed of the animation.
        @param useInt: If True, the attribute is always set to an integer value.
        @param onStop: Python callable to invoke when duration has elapsed and
        the animation has finished. This can be used to chain 
        animations together by using lambda to create a second animation.
        """
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop)
        g_Player.setTimeout(duration, self.__stop)
        self.interval = g_Player.setOnFrameHandler(self.__step)
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
    """
    Fades the opacity of a node to zero.
    @param node: The node to fade.
    @param duration: Length of the fade in milliseconds.
    """
    curValue = getattr(node, "opacity")
    LinearAnim(node, "opacity", duration, curValue, 0, 0, None)

def fadeIn(node, duration, max):
    """
    Fades the opacity of a node.
    @param node: The node to fade.
    @param duration: Length of the fade in milliseconds.
    @param max: The opacity of the node at the end of the fade.
    """
    curValue = getattr(node, "opacity")
    LinearAnim(node, "opacity", duration, curValue, max, 0, None)

def init(Player):
    global g_Player
    g_Player = Player
