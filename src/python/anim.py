# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de
#


import math

avg = None
g_Player = None

# Map of all active SimpleAnimations: (node, attribute)->SimpleAnimation
g_ActiveAnimations = {}

try:
    from . import avg
except ValueError:
    pass


def getNumRunningAnims():
    return len(g_ActiveAnimations)

def abortAnim(node, attrName):
    global g_ActiveAnimations
    if g_ActiveAnimations.has_key((node, attrName)):
        curAnim = g_ActiveAnimations.get((node, attrName))
        curAnim._remove()

g_DeprecationWarned = False

def deprecationWarning():
    global g_DeprecationWarned
    if not(g_DeprecationWarned):
        g_DeprecationWarned = True
        print "The anim package is deprecated and will be removed in the next release; use the anim classes in the avg namespace instead."

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
    def __init__(self, node, attrName, duration, useInt, onStop, onStart):
        global g_Player
        global g_ActiveAnimations

        deprecationWarning()
        g_Player = avg.Player.get()
        self.node = node
        self.attrName = attrName
        self.duration = duration
        self.onStart = onStart
        self.onStop = onStop
        self.onAbort = lambda: None
        self.useInt = useInt

    def setHandler(self, onStop, onAbort):
        self.onStop = onStop
        self.onAbort = onAbort

    def start(self, keepAttr=False):
        abortAnim(self.node, self.attrName)
        g_ActiveAnimations[(self.node, self.attrName)] = self
        if keepAttr:
            self._calcStartTime()
        else:
            self.startTime = g_Player.getFrameTime()
        self.__interval = g_Player.setOnFrameHandler(self._step)
        self.__done = False
        if self.onStart:
            self.onStart()
        if self.duration == 0:
            self._regularStop()
        elif self.duration:
            self.__stopTimeout = g_Player.setTimeout(self.duration, self._regularStop)
            self._step()

    def abort(self):
        """
        Stops the animation. Does not call onStop()
        """
        if not(self.isDone()):
            self._remove()
            if self.onAbort:
                self.onAbort()

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
        if self.duration:
            g_Player.clearInterval(self.__stopTimeout)


class LinearAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node by interpolating linearly
    between start and end values.
    """
    def __init__(self, node, attrName, duration, startValue, endValue, useInt=False, 
            onStop=None, onStart=None):
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
        self.__startValue = startValue
        self.__endValue = endValue
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop, onStart)

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

    def _calcStartTime(self):
        curVal = getattr(self.node, self.attrName)
        part = float(curVal-self.__startValue)/(self.__endValue-self.__startValue)
        self.startTime = g_Player.getFrameTime()-part*self.duration


class EaseInOutAnim(SimpleAnim):
    def __init__(self, node, attrName, duration, startValue, endValue, 
            easeInDuration, easeOutDuration, useInt=False, onStop=None, 
            onStart=None):
        self.__startValue = startValue
        self.__endValue = endValue
        self.__easeInDuration = float(easeInDuration)/duration
        self.__easeOutDuration = float(easeOutDuration)/duration
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop, onStart)

    def _step(self):
        if not(self.isDone()):
            t = (float(g_Player.getFrameTime())-self.startTime)/self.duration;
            part = self.__ease(t, self.__easeInDuration, self.__easeOutDuration)
            curValue = self.__startValue+(self.__endValue-self.__startValue)*part
            if self.useInt:
                curValue = int(curValue+0.5)
            setattr(self.node, self.attrName, curValue)

    def _regularStop(self):
        setattr(self.node, self.attrName, self.__endValue)
        self._remove()
        if self.onStop != None:
            self.onStop()

    def _calcStartTime(self):
        #XXX: This calculates an inaccurate start time 
        curVal = getattr(self.node, self.attrName)
        part = float(curVal-self.__startValue)/(self.__endValue-self.__startValue)
        self.startTime = g_Player.getFrameTime()-part*self.duration

    def __ease(self, t, easeInDuration, easeOutDuration):
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


class SplineAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node by interpolating 
    between start and end values using a cubic spline.
    """
    def __init__(self, node, attrName, duration, startValue, startSpeed, endValue, 
            endSpeed, useInt=False, onStop=None, onStart=None):
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
        self.__startValue = startValue+0.0
        self.__startSpeed = startSpeed
        self.__endValue = endValue
        self.__endSpeed = endSpeed
        self.__a = -2*(self.__endValue-self.__startValue)+self.__startSpeed+self.__endSpeed
        self.__b = 3*(self.__endValue-self.__startValue)-2*self.__startSpeed-self.__endSpeed
        self.__c = self.__startSpeed
        self.__d = self.__startValue
        SimpleAnim.__init__(self, node, attrName, duration, useInt, onStop, onStart)

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

    def _calcStartTime(self):
        #XXX: This calculates an inaccurate start time 
        curVal = getattr(self.node, self.attrName)
        part = float(curVal-self.__startValue)/(self.__endValue-self.__startValue)
        self.startTime = g_Player.getFrameTime()-part*self.duration


def fadeOut(node, duration, onStop = None):
    """
    Fades the opacity of a node to zero.
    @param node: The node to fade.
    @param duration: Length of the fade in milliseconds.
    """
    fader = LinearAnim(node, "opacity", duration, node.opacity, 0, onStop = onStop)
    fader.start()
    return fader 

def fadeIn(node, duration, max=1.0, onStop = None):
    """
    Fades the opacity of a node.
    @param node: The node to fade.
    @param duration: Length of the fade in milliseconds.
    @param max: The opacity of the node at the end of the fade.
    """
    fader = LinearAnim(node, "opacity", duration, node.opacity, max, onStop = onStop) 
    fader.start()
    return fader


class ContinuousAnim(SimpleAnim):
    """
    Class that animates an attribute of a libavg node continuously and
    linearly. The animation will not stop until the abort() method is called.
    A possible use case is the continuous rotation of an object.
    """
    def __init__(self, node, attrName, startValue, speed, useInt=False, 
            onStart=None):
        """
        @param node: The libavg node object to animate.
        @param attrName: The name of the attribute to change. Must be a numeric
        attribute.
        @param startValue: Initial value of the attribute.
        @param speed: Animation speed, value to be added per second.
        @param useInt: If True, the attribute is always set to an integer value.
        """
        self.__startValue = startValue
        self.__speed = speed
        SimpleAnim.__init__(self, node, attrName, None, useInt, None, onStart)

    def _step(self):
        time = (float(g_Player.getFrameTime())-self.startTime)/1000
        curValue = self.__startValue+time*self.__speed
        if self.useInt:
            curValue = int(curValue+0.5)
        setattr(self.node, self.attrName, curValue)

    def _calcStartTime(self):
        curVal = getattr(self.node, self.attrName)
        self.__startValue = curVal


class WaitAnim:
    def __init__(self, duration=None, onStop=None, onStart=None):
        self.__duration = duration
        self.onStart = onStart
        self.onStop = onStop
        self.onAbort = None
        self.__isDone = True

    def setHandler(self, onStop, onAbort):
        self.onStop = onStop
        self.onAbort = onAbort

    def start(self, keepAttr=False):
        self.__isDone = False
        if self.onStart:
            self.onStart()
        if self.__duration:
            self.__stopTimeout = g_Player.setTimeout(self.__duration, self.__regularStop)
        else:
            self.__stopTimeout = None

    def abort(self):
        if self.__stopTimeout:
            g_Player.clearInterval(self.__stopTimeout)
        if not(self.__isDone):
            self.__isDone = True
            if self.onAbort:
                self.onAbort()

    def isDone(self):
        return self.__isDone

    def _calcStartTime(self):
        pass

    def __regularStop(self):
        g_Player.clearInterval(self.__stopTimeout)
        self.__isDone = True
        self.onStop()


class ParallelAnim:
    def __init__(self, anims, onStop=None, onStart=None, maxAge=None):
        self.__anims = anims
        self.onStart = onStart
        self.onStop = onStop
        self.__maxAge = maxAge
        self.__isDone = False

    def setHandler(self, onStop, onAbort):
        self.onStop = onStop
        self.onAbort = onAbort

    def start(self, keepAttr=False):
        self.__isDone = False
        if self.onStart:
            self.onStart()
        self.__runningAnims = self.__anims[:]
        if self.__maxAge:
            self.__maxAgeTimeout = g_Player.setTimeout(self.__maxAge, 
                    self.__maxAgeReached)
        for anim in self.__runningAnims:
            stopHandler = lambda anim=anim: self.__animStopped(anim)
            anim.setHandler(onStop = stopHandler, onAbort = stopHandler)
            anim.start(keepAttr)

    def abort(self):
        if not(self.__isDone):
            self.__isDone = True
            for anim in self.__runningAnims:
                anim.abort()
            if self.onAbort:
                self.onAbort()
            if self.__maxAge:
                g_Player.clearInterval(self.__maxAgeTimeout)

    def isDone(self):
        return self.__isDone

    def __maxAgeReached(self):
        if not(self.__isDone):
            for anim in self.__runningAnims:
                anim.abort()
            self.onStop()
            self.__isDone = True

    def __animStopped(self, anim):
        self.__runningAnims.remove(anim)
        if len(self.__runningAnims) == 0 and not(self.__isDone):
            self.onStop()
            self.__isDone = True
            if self.__maxAge:
                g_Player.clearInterval(self.__maxAgeTimeout)


class StateAnim:
    def __init__(self, states, transitions, initialState=None):
        self.__states = states
        for name in states:
            states[name].setHandler(self.__onStateDone, None)
        self.__transitions = transitions
        self.__curState = None
        self.__debug = False
        if initialState:
            self.setState(initialState)

    def delete(self):
        if self.__debug:
            print self, " delete"
        self.setState(None)

    def setState(self, stateName, keepAttr=False):
        if self.__debug:
            print self, " setState: ", self.__curState, "-->", stateName
        if self.__curState == stateName:
            return
        if self.__curState:
            self.__states[self.__curState].abort()
        self.__curState = stateName
        if stateName:
            self.__states[stateName].start(keepAttr)

    def getState(self):
        return self.__curState

    def setDebug(self, debug):
        self.__debug = debug

    def __onStateDone(self):
        if self.__curState in self.__transitions:
            transition = self.__transitions[self.__curState]
            if transition.callback:
                transition.callback()
            stateName = transition.nextAnimName
            if self.__debug:
                print self, " StateDone: ", self.__curState, "-->", stateName
            self.__curState = stateName
            self.__states[stateName].start()
        else:
            if self.__debug:
                print self, " StateDone: ", self.__curState, "--> None"
            self.__curState = None


class AnimTransition:
    def __init__(self, nextAnimName, callback = None):
        self.nextAnimName = nextAnimName
        self.callback = callback


def init(g_avg):
    global avg
    global g_ActiveAnimations
    avg = g_avg
    g_ActiveAnimations = {}
