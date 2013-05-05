# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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

from methodref import methodref

import subprocess
import os

class State(object):
    def __init__(self, transitions, enterFunc, leaveFunc):
        self.transitions = {}
        for destState, transfunc in transitions.items():
            ref = methodref(transfunc)
            self.transitions[destState] = ref
        self.enterFunc = methodref(enterFunc)
        self.leaveFunc = methodref(leaveFunc)

class StateMachine(object):
    def __init__(self, name, startState):
        self.__states = {}
        self.__name = name
        self.__startState = startState
        self.__curState = startState
        self.__trace = False
        self.__initDone = False

    def addState(self, state, transitions, enterFunc=None, leaveFunc=None):
        if self.__initDone:
            raise RuntimeError(
                    "StateMachine: Can't add new states after calling changeState")
        if self.__states.has_key(state):
            raise RuntimeError("StateMachine: Duplicate state " + state + ".")

        if isinstance(transitions, (list, tuple)):
            transitions = dict.fromkeys(transitions)
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
            if state.leaveFunc() != None:
                state.leaveFunc()()
            transitionFunc = state.transitions[newState]()
            if transitionFunc != None:
                try:
                    transitionFunc(self.__curState, newState)
                except TypeError:
                    transitionFunc()
            self.__curState = newState
            enterFunc = self.__states[self.__curState].enterFunc()
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
        for oldStateName, state in self.__states.iteritems():
            print oldStateName, ("(enter: " + self.__getNiceFuncName(state.enterFunc)
                    + ", leave: " + self.__getNiceFuncName(state.leaveFunc) + "):")
            for newState, func in state.transitions.iteritems():
                print "  -->", newState, ":", self.__getNiceFuncName(func)
        print "Current state:", self.__curState

    def makeDiagram(self, fName, showMethods=False):
        def writeState(stateName, state):
            label = stateName
            if showMethods:
                if state.enterFunc.__name__ is not(None):
                    label += ('<br/><font point-size="10">entry/' 
                            + state.enterFunc.__name__ + '</font>')
                if state.leaveFunc.__name__ is not(None):
                    label += ('<br/><font point-size="10">exit/' 
                            + state.leaveFunc.__name__ +  '</font>')
            dotFile.write('    "'+stateName+'" [label=<'+label+'>];\n')

        def writeTransition(origState, destState, func):
            dotFile.write('    "'+origState+'" -> "'+destState+'"')
            if showMethods and func and func.__name__ is not(None):
                dotFile.write(' [label="/'+func.__name__+'", fontsize=10]')
            dotFile.write(";\n")
            

        dotFile = open("tmp.dot", "w")
        dotFile.write('digraph "'+self.__name+'" {\n')
        dotFile.write('    node [fontsize=12, shape=box, style=rounded];\n')
        dotFile.write('    startstate [shape=point, height=0.2, width=0.2, label=""];\n')
        dotFile.write('    { rank=source; "startstate" };\n')
        writeTransition("startstate", self.__startState, None)
        for stateName, state in self.__states.iteritems():
            writeState(stateName, state)
            for destState, func in state.transitions.iteritems():
                writeTransition(stateName, destState, func)
        dotFile.write('    "'+self.__curState+'" [style="rounded,bold"];\n')
        dotFile.write('}\n')
        dotFile.close()
        try:
            subprocess.call(["dot", "tmp.dot", "-Tpng", "-o"+fName])
        except OSError:
            raise RuntimeError("dot executable not found. graphviz needs to be installed for StateMachine.makeDiagram to work.")
        os.remove("tmp.dot")

    def __getNiceFuncName(self, f):
        if f.__name__ is not(None):
            return f.__name__
        else:
            return "None"

    def __doSanityCheck(self):
        for stateName, state in self.__states.iteritems():
            for transitionName in state.transitions.iterkeys():
                if not(self.__states.has_key(transitionName)):
                    raise RuntimeError("StateMachine: transition " + stateName + " -> " + 
                            transitionName + " has an unknown destination state.")
