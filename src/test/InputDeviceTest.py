# -*- coding: utf-8 -*-
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

from libavg import avg, player
from testcase import *


class CustomMouseEvent(avg.TouchEvent):
    def __init__(self, eventId, eventType, pos, source):
        super(CustomMouseEvent, self).__init__(eventId, eventType, pos, source)
        self.customAttribute = None
    
    def customMethod(self):
        pass


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
        return [ avg.Event(avg.Event.CUSTOM_EVENT, avg.Event.CUSTOM) ]


class EventTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)
    
    def testCustomInputDevice(self):
        root = self.loadEmptyScene()

        class DerivedEvent(avg.Event):
            def __init__(self):
                super(DerivedEvent, self).__init__(avg.Event.CUSTOM_EVENT, avg.Event.NONE)
                self.property = True
                
        class CustomMouseEvent(avg.TouchEvent):
            def __init__(self):
                super(CustomMouseEvent, self).__init__(42, avg.Event.CURSOR_DOWN, 
                        (10, 10), avg.Event.TOUCH)
                self.customAttribute = None

        self.hasEventHandlerBeenCalled = False
        self.isCustomInputDeviceSet = False
        self.isCustomInputDeviceNameSet = False
        self.hasCustomEventProperty = False
        self.customMouseEventHandlerCalled = False
        
        def eventHandler(event):
            self.hasEventHandlerBeenCalled = True
            self.isCustomInputDeviceSet = event.inputdevice == self.customInputDevice
            self.isCustomInputDeviceNameSet = (event.inputdevicename == 
                    self.customInputDevice.name)
        
        def customEventEventHandler(event):
            self.hasCustomEventProperty = event.property

        def customMouseEventHandler(event):
            self.customMouseEventHandlerCalled = True
            self.assert_(hasattr(event, "customAttribute"))

        def checkAndResetResults():
            if not self.hasEventHandlerBeenCalled: return False
            if not self.isCustomInputDeviceSet: return False
            if not self.isCustomInputDeviceNameSet: return False
            
            self.hasEventHandlerBeenCalled = False
            self.isCustomInputDeviceSet = False
            self.isCustomInputDeviceNameSet = False
            return True
        
        rectNode = avg.RectNode(parent=root, pos=(0, 0), size=(50, 50))
        rectNode.subscribe(avg.Node.CURSOR_DOWN, eventHandler)
        
        root.subscribe(avg.Node.CURSOR_DOWN, eventHandler)
        root.setEventHandler(avg.Event.CUSTOM_EVENT, avg.Event.NONE, 
                customEventEventHandler)
        
        self.customInputDevice = CustomInputDevice()
        player.addInputDevice(self.customInputDevice)
    
        self.start(False,
                (lambda: self.customInputDevice.feedEvent(
                         avg.Event(avg.Event.CURSOR_DOWN, avg.Event.MOUSE)),
                 lambda: self.assert_(checkAndResetResults()),
   
                 lambda: self.customInputDevice.feedEvent(
                         DerivedEvent()),
                 lambda: self.assert_(self.hasCustomEventProperty),
                 
                 lambda: self.customInputDevice.feedEvent(
                         avg.MouseEvent(avg.Event.CURSOR_DOWN, False, False, False,
                                (5, 5), 0)),
                 lambda: self.assert_(checkAndResetResults()),
                 
                 lambda: self.customInputDevice.feedEvent(
                         avg.TouchEvent(300, avg.Event.CURSOR_DOWN, (5, 5), 
                                avg.Event.TOUCH, (10,10))),
                 lambda: self.assert_(checkAndResetResults()),

                 lambda: root.subscribe(avg.Node.CURSOR_DOWN, customMouseEventHandler),
                 lambda: self.customInputDevice.feedEvent(CustomMouseEvent()),
                 lambda: self.assert_(self.customMouseEventHandlerCalled),
                ))

    def testAnonymousInputDevice(self):
        root = self.loadEmptyScene()
        
        self.hasEventHandlerBeenCalled = False
                
        def eventHandler(event):
            self.hasEventHandlerBeenCalled = (event.inputdevicename ==
                    AnonymousInputDevice.__name__)
            
        def checkAndResetResults():
            if not self.hasEventHandlerBeenCalled: 
                return False
            
            self.hasEventHandlerBeenCalled = False
            return True
        
        root.setEventHandler(avg.Event.CUSTOM_EVENT, avg.Event.CUSTOM, eventHandler)
        player.addInputDevice(AnonymousInputDevice())
    
        self.start(False,
                (lambda: None,
                 lambda: None,
                 lambda: self.assert_(checkAndResetResults())
                ))

    def testInputDeviceEventReceiverNode(self):
        root = self.loadEmptyScene()

        divNode = avg.DivNode(id="div", size=(50, 50), parent=root)
        rectNode = avg.RectNode(id="rect", size=(50, 50), parent=root)
        
        self.customInputDevice = CustomInputDevice(divNode)
        player.addInputDevice(self.customInputDevice)
    
        handlerTester = NodeHandlerTester(self, divNode)

        self.start(False,
                (lambda: self.customInputDevice.feedEvent(
                        avg.MouseEvent(avg.Event.CURSOR_DOWN, True, False, False,
                                (10, 10), 1)),
                 lambda: handlerTester.assertState(
                        (avg.Node.CURSOR_DOWN, avg.Node.CURSOR_OVER)),
                 
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.Event.CURSOR_MOTION, True, False, False, (12, 12), 1)),
                 lambda: handlerTester.assertState((avg.Node.CURSOR_MOTION,)),
                 
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.Event.CURSOR_MOTION, True, False, False, (100, 100), 1)),
                 lambda: handlerTester.assertState((avg.Node.CURSOR_OUT,)),
                 
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.Event.CURSOR_MOTION, True, False, False, (12, 12), 1)),
                 lambda: handlerTester.assertState(
                        (avg.Node.CURSOR_OVER, avg.Node.CURSOR_MOTION)),
                        
                 lambda: self.customInputDevice.feedEvent(avg.MouseEvent(
                         avg.Event.CURSOR_UP, False, False, False, (12, 12), 1)),
                 lambda: handlerTester.assertState((avg.Node.CURSOR_UP,)),
                ))

def inputDeviceTestSuite(tests):
    availableTests = (
            "testCustomInputDevice",
            "testAnonymousInputDevice",
            "testInputDeviceEventReceiverNode"
    )
    return createAVGTestSuite(availableTests, EventTestCase, tests)
