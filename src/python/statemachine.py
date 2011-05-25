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

class StateMachine:
    def __init__(self, startState):
        self.__states = {}
        self.__curState = startState

    def addState(self, state, transitions):
        self.__states[state] = transitions

    def changeState(self, newState):
        if not(newState in self.__states):
            raise RuntimeError('StateMachine: Attempt to change to nonexistent state '+
                    newState+'.')
        assert(self.__curState in self.__states)
        curTransitions = self.__states[self.__curState]
        if newState in curTransitions:
            transitionFunc = curTransitions[newState]
            if transitionFunc != None:
                try:
                    transitionFunc(self.__curState, newState)
                except TypeError:
                    transitionFunc()
            self.__curState = newState
        else:
            raise RuntimeError('StateMachine: State change from '+self.__curState+' to '+
                    newState+' not allowed.')

    @property
    def state(self):
        return self.__curState

    def dump(self):
        for oldState, transitions in self.__states.iteritems():
            print oldState, ":"
            for newState, func in transitions.iteritems():
                print "  -->", newState, ":", func.__name__
        print "Current state:", self.__curState

