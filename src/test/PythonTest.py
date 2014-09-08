# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

import os
import time
import tempfile

from libavg import geom, statemachine, persist

from testcase import *

def getTempFileName():
    return os.path.join(tempfile.gettempdir(), 'libavg.%d' % time.time())


class PythonTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testRoundedRect(self):
        def setPos():
            self.rect.pos = (20.5, 3.5)

        def setSize():
            self.rect.size = (40, 20)

        def setRadius(r):
            self.rect.radius = r

        def setFill():
            self.rect.fillcolor = "0000FF"
            self.rect.fillopacity = 0.5

        def createDegenRect():
            self.rect.unlink(True)
            rect = geom.RoundedRect(parent=root, pos=(10.5,10.5), size=(10,10), radius=6, 
                    fillopacity=0.5, color="FFFFFF")
            self.assert_(rect.radius == 6)

        root = self.loadEmptyScene()
        self.rect = geom.RoundedRect(parent=root, pos=(2.5,2.5), 
                size=(64,64), radius=5, color="FF0000")
        self.start(False,
                (lambda: self.compareImage("testRoundedRect1"),
                 setPos,
                 lambda: self.compareImage("testRoundedRect2"),
                 setSize,
                 lambda: self.compareImage("testRoundedRect3"),
                 lambda: setRadius(10),
                 lambda: self.compareImage("testRoundedRect4"),
                 setFill,
                 lambda: self.compareImage("testRoundedRect5"),
                 createDegenRect,
                 lambda: self.compareImage("testRoundedRect6"),
                ))

    def testPieSlice(self):
        def changeAttrs():
            self.pieSlice.startangle = -1
            self.pieSlice.endangle = 3.14
            self.pieSlice.radius = 50
            self.pieSlice.pos = (80.5, 60.5)
            self.pieSlice.fillcolor = "00FFFF"
            self.pieSlice.fillopacity = 0.5

        def makeSmall():
            self.pieSlice.radius = 0.6

        root = self.loadEmptyScene()
        self.pieSlice = geom.PieSlice(parent=root, pos=(20.5,20.5), 
                radius=40, startangle=0, endangle=1.57, color="FF0000")
       
        self.start(False,
                (lambda: self.compareImage("testPieSlice1"),
                 changeAttrs,
                 lambda: self.compareImage("testPieSlice2"),
                 makeSmall,
                 lambda: self.compareImage("testPieSlice3"),
                ))

    def testArc(self):
        def changeAttrs():
            self.arc.startangle = -1
            self.arc.endangle = 3.14
            self.arc.radius = 50
            self.arc.pos = (80.5, 60.5)

        root = self.loadEmptyScene()
        self.arc = geom.Arc(parent=root, pos=(20.5,20.5), 
                radius=40, startangle=0, endangle=1.57, color="FF0000")
       
        self.start(False,
                (lambda: self.compareImage("testArc1"),
                 changeAttrs,
                 lambda: self.compareImage("testArc2"),
                ))

    def btoa(self):
        # Test for member function handling in StateMachine.
        self.btoaCalled = True

    def testStateMachine(self):
        def atob(oldState, newState):
            self.atobCalled = True

        def btoc(dummy):
            # Dummy argument so we can test handling of lambda expressions.
            self.btocCalled = True

        def aEntered():
            self.aEnteredCalled = True

        def aLeft():
            self.aLeftCalled = True

        self.atobCalled = False
        self.btocCalled = False
        self.btoaCalled = False
        self.aLeftCalled = False
        self.aEnteredCalled = False
        machine = statemachine.StateMachine("testmachine", 'A')
        machine.addState('A', {'B': atob, 'nostate': atob}, aEntered, aLeft)
        machine.addState('B', {'C': lambda: btoc("dummy"), 'A': self.btoa})
        machine.addState('C', {'A': None})
        self.assertRaises(RuntimeError, lambda: machine.addState('C', {'A': None}))
        self.assertRaises(RuntimeError, lambda: machine.changeState('C'))
        self.assertRaises(RuntimeError, lambda: machine.changeState('nostate'))
        machine.changeState('B')
        self.assert_(self.atobCalled)
        self.assert_(self.aLeftCalled)
        machine.changeState('A')
        self.assert_(self.aEnteredCalled)
        self.assert_(self.btoaCalled)
        machine.changeState('B')
        machine.changeState('C')
        self.assert_(self.btocCalled)
        machine.changeState('A')
        self.assertEqual(machine.state, 'A')
#        machine.dump()

        self.assertRaises(RuntimeError, lambda: machine.addState('illegal', {}))

        # Create a machine without transition callbacks
        machine = statemachine.StateMachine("testmachine", 'A')
        machine.addState('A', ('B',), aEntered, aLeft)
        machine.addState('B', ('C', 'A'))
        machine.addState('C', ('A',))
        machine.changeState('B')

        # Make a machine with a transition to a nonexistent state.
        kaputtMachine = statemachine.StateMachine("kaputt", 'A')
        kaputtMachine.addState('A', {'B': None})
        self.assertRaises(RuntimeError, lambda: kaputtMachine.changeState('B'))

    def testStateMachineDiagram(self):
        def aEntered():
            pass

        if not(self._isCurrentDirWriteable()):
            self.skip("Current dir not writeable")
            return
        
        machine = statemachine.StateMachine("testmachine", 'A')
        machine.addState('A', {'B': None, 'nostate': None}, aEntered)
        machine.addState('B', {'C': None, 'A': self.btoa})
        machine.addState('C', {'A': None})

        imageFName = AVGTestCase.imageResultDirectory + "/stateMachineGraphViz.png"
        try:
            machine.makeDiagram(imageFName)
        except RuntimeError:
            self.skip("graphviz not installed.")

    def testPersistStore(self):
        testFile = getTempFileName()
        p = persist.Persist(testFile, {})
        self.assertEqual(p.storeFile, testFile)
        self.assertEqual(p.data, {})
        p.data['test'] = 1
        p.commit()
        p = persist.Persist(testFile, {})
        self.assert_('test' in p.data)
        self.assertEqual(p.data['test'], 1)
        os.unlink(testFile)
        
    def testPersistCorrupted(self):
        logger.configureCategory("APP", logger.Severity.ERR);
        testFile = getTempFileName()
        f = open(testFile, 'w')
        f.write('garbage')
        f.close()
        p = persist.Persist(testFile, {})
        self.assertEqual(p.data, {})
        os.unlink(testFile)
        logger.configureCategory("APP", logger.Severity.WARN);

    def testPersistValidation(self):
        logger.configureCategory("APP", logger.Severity.ERR);
        testFile = getTempFileName()
        p = persist.Persist(testFile, {'test': 1})
        p.commit()
        p = persist.Persist(testFile, [], validator=lambda v: isinstance(v, list))
        self.assertEqual(p.data, [])
        os.unlink(testFile)
        logger.configureCategory("APP", logger.Severity.WARN);


def pythonTestSuite(tests):
    availableTests = (
        "testRoundedRect",
        "testPieSlice",
        "testArc",
        "testStateMachine",
        "testStateMachineDiagram",
        "testPersistStore",
        "testPersistCorrupted",
        "testPersistValidation",
        )
    
    return createAVGTestSuite(availableTests, PythonTestCase, tests)

