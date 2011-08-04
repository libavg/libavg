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

class State:
    def __init__(self, transitions, enterFunc, leaveFunc):
        self.transitions = transitions
        self.enterFunc = enterFunc
        self.leaveFunc = leaveFunc

class StateMachine:
    def __init__(self, name, startState):
        self.__states = {}
        self.__name = name
        self.__curState = startState
        self.__trace = False
        self.__initDone = False

    def addState(self, state, transitions, enterFunc=None, leaveFunc=None):
        if self.__initDone:
            raise RuntimeError(
                    "StateMachine: Can't add new states after calling changeState")
        if self.__states.has_key(state):
            raise RuntimeError("StateMachine: Duplicate state " + state + ".")

        self.__states[state] = State(transitions, enterFunc, leaveFunc)

    def changeState(self, newState):
        if not(self.__initDone):
            self.__initDone = True
            self.__doSanityCheck()

        if self.__trace:
            print self.__name, ":", self.__curState, "-->", newState

        if not(newState in self.__states):
            raise RuntimeError('StateMachine: Attempt to change to nonexistent state '+
                    newState+'.')
        assert(self.__curState in self.__states)
        state = self.__states[self.__curState]
        if newState in state.transitions:
            if state.leaveFunc != None:
                state.leaveFunc()
            transitionFunc = state.transitions[newState]
            if transitionFunc != None:
                try:
                    transitionFunc(self.__curState, newState)
                except TypeError:
                    transitionFunc()
            self.__curState = newState
            enterFunc = self.__states[self.__curState].enterFunc
            if enterFunc != None:
                enterFunc()
        else:
            raise RuntimeError('StateMachine: State change from '+self.__curState+' to '+
                    newState+' not allowed.')

    def traceChanges(self, trace):
        self.__trace = trace

    @property
    def state(self):
        return self.__curState

    def dump(self):
        for oldState, transitions in self.__states.iteritems():
            print oldState, ":"
            for newState, func in transitions.iteritems():
                print "  -->", newState, ":", func.__name__
        print "Current state:", self.__curState

    def __doSanityCheck(self):
        for stateName, state in self.__states.iteritems():
            for transitionName in state.transitions.iterkeys():
                if not(self.__states.has_key(transitionName)):
                    raise RuntimeError("StateMachine: transition " + stateName + " -> " + 
                            transitionName + " has an unknown destination state.")
