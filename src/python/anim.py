# TODO:
# - loops
# - Folgen, Gruppen

import math

avg = None
g_Player = None

g_ActiveAnimations = {}


try:
    from . import avg
except ValueError:
    pass


def getNumRunningAnims():
    return len(g_ActiveAnimations)

class SimpleAnim:
    """
    Base class for animations that change libavg node attributes by interpolating
    over a set amount of time. Constructing an animation object starts the
    animation. If abort() isn't needed, there is no need to hold on to the object -
    it will exist exactly as long as the animation lasts and then disappear.

    The animation framework makes sure that only one animation per attribute of a
    node runs at any given time. If a second one is started, the first one is 
    silently aborted.
    """
    def __init__(self, node, attrName, duration, useInt, onStop):
        global g_Player
        global g_ActiveAnimations
        if g_ActiveAnimations.has_key((node, attrName)):
            oldAnim = g_ActiveAnimations.get((node, attrName))
            oldAnim._remove()
        g_ActiveAnimations[(node, attrName)] = self

        g_Player = avg.Player.get()
        self.node = node
        self.attrName = attrName
        self.duration = duration
        self.startTime = g_Player.getFrameTime()
        self.onStop = onStop
        self.useInt = useInt
        if duration != 0:
            self.__stopTimeout = g_Player.setTimeout(duration, self._regularStop)
        self.__interval = g_Player.setOnFrameHandler(self._step)
        self.__done = False
    def abort(self):
        """
        Stops the animation. Does not call onStop()
        """
        if not(self.isDone()):
            self._remove()
    def isDone(self):
        """
        Returns True if the animation has run its course.
        """
        return self.__done
    def _remove(self):
        global g_ActiveAnimations
        self.__done = True
        g_ActiveAnimations.pop((self.node, self.attrName))
        g_Player.clearInterval(self.__interval)
        if self.duration != 0:
            g_Player.clearInterval(self.__stopTimeout)

class LinearAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node by interpolating linearly
    between start and end values.
    """
    def __init__(self, node, attrName, duration, startValue, endValue, useInt=False, onStop=None):
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
        self.__startValue = startValue
        self.__endValue = endValue
        self._step()
    def _step(self):
        if not(self.isDone()):
            part = ((float(g_Player.getFrameTime())-self.startTime)/self.duration)
            if part > 1.0:
                part = 1.0
            curValue = self.__startValue+(self.__endValue-self.__startValue)*part
            if self.useInt:
                curValue = int(curValue+0.5)
            setattr(self.node, self.attrName, curValue)
    def _regularStop(self):
        setattr(self.node, self.attrName, self.__endValue)
        self._remove()
        if self.onStop != None:
            self.onStop()

class EaseInOutAnim(SimpleAnim):
    def __init__(self, node, attrName, duration, startValue, endValue, 
            easeInDuration, easeOutDuration, useInt=False, onStop=None):
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop)
        self.__startValue = startValue
        self.__endValue = endValue
        self.__easeInDuration = float(easeInDuration)/duration
        self.__easeOutDuration = float(easeOutDuration)/duration
        self._step()
    def _step(self):
        def ease(t, easeInDuration, easeOutDuration):
            # All times here are normalized to be between 0 and 1
            if t > 1:
                t=1
            accelDist = easeInDuration*2/math.pi
            decelDist = easeOutDuration*2/math.pi
            if t<easeInDuration:
                # Acceleration stage 
                nt=t/easeInDuration
                s=math.sin(-math.pi/2+nt*math.pi/2)+1;
                dist=s*accelDist;
            elif t > 1-easeOutDuration:
                # Deceleration stage
                nt = (t-(1-easeOutDuration))/easeOutDuration
                s = math.sin(nt*math.pi/2)
                dist = accelDist+(1-easeInDuration-easeOutDuration)+s*decelDist
            else:
                # Linear stage
                dist = accelDist+t-easeInDuration
            return dist/(accelDist+(1-easeInDuration-easeOutDuration)+decelDist)
        if not(self.isDone()):
            t = (float(g_Player.getFrameTime())-self.startTime)/self.duration;
            part = ease(t, self.__easeInDuration, self.__easeOutDuration)
            curValue = self.__startValue+(self.__endValue-self.__startValue)*part
            if self.useInt:
                curValue = int(curValue+0.5)
            setattr(self.node, self.attrName, curValue)
    def _regularStop(self):
        setattr(self.node, self.attrName, self.__endValue)
        self._remove()
        if self.onStop != None:
            self.onStop()

class SplineAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node by interpolating 
    between start and end values using a cubic spline.
    """
    def __init__(self, node, attrName, duration, 
            startValue, startSpeed, endValue, endSpeed, useInt=False, onStop=None):
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
        self.__startValue = startValue+0.0
        self.__startSpeed = startSpeed
        self.__endValue = endValue
        self.__endSpeed = endSpeed
        self.__a = -2*(self.__endValue-self.__startValue)+self.__startSpeed+self.__endSpeed
        self.__b = 3*(self.__endValue-self.__startValue)-2*self.__startSpeed-self.__endSpeed
        self.__c = self.__startSpeed
        self.__d = self.__startValue
        self._step()
    def _step(self):
        if not(self.isDone()):
            part = ((float(g_Player.getFrameTime())-self.startTime)/self.duration)
            if part > 1.0:
                part = 1.0
            curValue = ((self.__a*part+self.__b)*part+self.__c)*part+self.__d
            if self.useInt:
                curValue = int(curValue+0.5)
            setattr(self.node, self.attrName, curValue)
    def _regularStop(self):
        setattr(self.node, self.attrName, self.__endValue)
        self._remove()
        if self.onStop != None:
            self.onStop()

def fadeOut(node, duration):
    """
    Fades the opacity of a node to zero.
    @param node: The node to fade.
    @param duration: Length of the fade in milliseconds.
    """
    curValue = getattr(node, "opacity")
    return LinearAnim(node, "opacity", duration, curValue, 0)

def fadeIn(node, duration, max=1.0):
    """
    Fades the opacity of a node.
    @param node: The node to fade.
    @param duration: Length of the fade in milliseconds.
    @param max: The opacity of the node at the end of the fade.
    """
    curValue = getattr(node, "opacity")
    return LinearAnim(node, "opacity", duration, curValue, max)


class ContinuousAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node continuously and
    linearly. The animation will not stop until the abort() method is called.
    A possible use case is the continuous rotation of an object.

    """
    def __init__(self, node, attrName, startValue, speed, useInt=False):
        """
        @param node: The libavg node object to animate.
        @param attrName: The name of the attribute to change. Must be a numeric
        attribute.
        @param startValue: Initial value of the attribute.
        @param speed: Animation speed, value to be added per second.
        @param useInt: If True, the attribute is always set to an integer value.
        """
        SimpleAnim.__init__(self, node, attrName, 0, useInt, None)
        self.__startValue = startValue
        self.__speed = speed
        self._step()
    def _step(self):
        time = (float(g_Player.getFrameTime())-self.startTime)/1000
        curValue = self.__startValue+time*self.__speed
        if self.useInt:
            curValue = int(curValue+0.5)
        setattr(self.node, self.attrName, curValue)

def init(g_avg):
    global avg
    global g_ActiveAnimations
    avg = g_avg
    g_ActiveAnimations = {}

