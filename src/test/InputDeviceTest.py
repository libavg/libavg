# -*- coding: utf-8 -*-
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

import unittest

from libavg import avg
from testcase import *


class CustomInputDevice(avg.InputDevice):
    def __init__(self, eventReceiverNode=None):
        if eventReceiverNode:
            super(CustomInputDevice, self).__init__(self.__class__.__name__, 
                    eventReceiverNode)
        else:
            super(CustomInputDevice, self).__init__(self.__class__.__name__)
            
        self.__events = []
    
    def pollEvents(self):
        events = self.__events[:]
        self.__events = []
        return events

    def feedEvent(self, event):
        self.__events.append(event)
      

class AnonymousInputDevice(avg.InputDevice):
    def __init__(self, eventReceiverNode=None):
        if eventReceiverNode:
            super(AnonymousInputDevice, self).__init__(self.__class__.__name__, 
                    eventReceiverNode)
        else:
            super(AnonymousInputDevice, self).__init__(self.__class__.__name__)
            
        self.__isInitialized = False

    def pollEvents(self):
        if self.__isInitialized: return []
        self.__isInitialized = True
        return [ avg.Event(avg.CUSTOMEVENT, avg.CUSTOM) ]


class EventTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)
    
    def testCustomInputDevice(self):
        self.loadEmptyScene()

        class DerivedEvent(avg.Event):
            def __init__(self):
                super(DerivedEvent, self).__init__(avg.CUSTOMEVENT, avg.NONE)
                self.property = True
                
        self.hasEventHandlerBeenCalled = False
        self.isCustomInputDeviceSet = False
        self.isCustomInputDeviceNameSet = False
        self.hasCustomEventProperty = False
        
        def eventHandler(event):
            self.hasEventHandlerBeenCalled = True
            self.isCustomInputDeviceSet = event.inputdevice == self.customInputDevice
            self.isCustomInputDeviceNameSet = (event.inputdevicename == 
                    self.customInputDevice.name)
        
        def customEventEventHandler(event):
            self.hasCustomEventProperty = event.property
        
        def checkAndResetResults():
            if not self.hasEventHandlerBeenCalled: return False
            if not self.isCustomInputDeviceSet: return False
            if not self.isCustomInputDeviceNameSet: return False
            
            self.hasEventHandlerBeenCalled = False
            self.isCustomInputDeviceSet = False
            self.isCustomInputDeviceNameSet = False
            return True
        
        rectNode = avg.RectNode(parent=Player.getRootNode(), pos=(0, 0), size=(50, 50))
        rectNode.setEventHandler(avg.CURSORDOWN, avg.MOUSE|avg.TOUCH, eventHandler)
        
        Player.getRootNode().setEventHandler(avg.CURSORDOWN, avg.NONE, eventHandler)
        Player.getRootNode().setEventHandler(avg.CUSTOMEVENT, avg.NONE, customEventEventHandler)
        
        self.customInputDevice = CustomInputDevice()
        Player.addInputDevice(self.customInputDevice)
    
        self.start(( 
                 lambda: self.customInputDevice.feedEvent(
                         avg.Event(avg.CURSORDOWN, avg.NONE)),
                 lambda: self.assert_(checkAndResetResults()),
   
                 lambda: self.customInputDevice.feedEvent(
                         DerivedEvent()),
                 lambda: self.assert_(self.hasCustomEventProperty),
                 
                 lambda: self.customInputDevice.feedEvent(
                         avg.MouseEvent(avg.CURSORDOWN, False, False, False, (5, 5), 0)),
                 lambda: self.assert_(checkAndResetResults()),
                 
                 lambda: self.customInputDevice.feedEvent(
                         avg.TouchEvent(300, avg.CURSORDOWN, (5, 5), avg.TOUCH, (10,10))),
                 lambda: self.assert_(checkAndResetResults())
        ))
    
    def testAnonymousInputDevice(self):
        self.loadEmptyScene()
        
        self.hasEventHandlerBeenCalled = False
                
        def eventHandler(event):
            self.hasEventHandlerBeenCalled = (event.inputdevicename ==
                    AnonymousInputDevice.__name__)
            
        def checkAndResetResults():
            if not self.hasEventHandlerBeenCalled: 
                return False
            
            self.hasEventHandlerBeenCalled = False
            return True
        
        Player.getRootNode().setEventHandler(avg.CUSTOMEVENT, avg.CUSTOM, eventHandler)
        Player.addInputDevice(AnonymousInputDevice())
    
        self.start(( 
                 lambda: None,
                 lambda: None,
                 lambda: self.assert_(checkAndResetResults())
        ))

    def testInputDeviceEventReceiverNode(self):
        self.loadEmptyScene()

        divNode = avg.DivNode(id="div", size=(50, 50), parent=Player.getRootNode())
        rectNode = avg.RectNode(id="rect", size=(50, 50), parent=Player.getRootNode())
        
        self.customInputDevice = CustomInputDevice(divNode)
        Player.addInputDevice(self.customInputDevice)
    
        handlerTester = NodeHandlerTester(self, divNode)

        self.start(( 
                 lambda: self.customInputDevice.feedEvent(
                        avg.MouseEvent(avg.CURSORDOWN, True, False, False, (10, 10), 1)),
                 lambda: handlerTester.assertState(
                        down=True, up=False, over=True, out=False, move=False),
                 
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.CURSORMOTION, True, False, False, (12, 12), 1)),
                 lambda: handlerTester.assertState(
                        down=False, up=False, over=False, out=False, move=True),
                 
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.CURSORMOTION, True, False, False, (100, 100), 1)),
                 lambda: handlerTester.assertState(
                        down=False, up=False, over=False, out=True, move=False),
                 
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.CURSORMOTION, True, False, False, (12, 12), 1)),
                 lambda: handlerTester.assertState(
                        down=False, up=False, over=True, out=False, move=True),
                        
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.CURSORUP, False, False, False, (12, 12), 1)),
                 lambda: handlerTester.assertState(
                        down=False, up=True, over=False, out=False, move=False)
                 
                ))

def inputDeviceTestSuite(tests):
    availableTests = (
            "testCustomInputDevice",
            "testAnonymousInputDevice",
            "testInputDeviceEventReceiverNode"
    )
    return createAVGTestSuite(availableTests, EventTestCase, tests)

Player = avg.Player.get()
Helper = Player.getTestHelper()
