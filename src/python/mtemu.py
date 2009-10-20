#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2009 Archimedes Solutions GmbH,
# Saarbrücker Str. 24b, Berlin, Germany
#
# This file contains proprietary source code and confidential
# information. Its contents may not be disclosed or distributed to
# third parties unless prior specific permission by Archimedes
# Solutions GmbH, Berlin, Germany is obtained in writing. This applies
# to copies made in any form and using any medium. It applies to
# partial as well as complete copies.

"""
this class provides a test to emulate one or two TOUCH/TRACK events. 
by pressing "ctrl left" the TOUCH events will be switched into TRACK events and the other way around.
first TOUCH/TRACK should be created with a left mouse button. after that you can create another by 
pressing the right button. 
the second event appears at the position:   mousePosition - (50,50) pixels.

Note: remove any mouse event handling from your application to avoid emulation issues

For example:

1) avg.MOUSE | avg.TOUCH filters can be safely reduced to avg.TOUCH
2) the current libavg button module breaks the emulation
3) the current libavg Grabbable class breaks the emulation, unless is created with source = avg.TOUCH parameter
"""

from libavg import avg, Point2D

Player = avg.Player.get()

class MTemu(object): 
    """
    this class provides a test method to
    emulate one or two TOUCH events. 
    by pressing "ctrl left" the TOUCH events 
    will be switched into TRACK events.
    
    ID behaviour:
    left clicks have ID up from 10000
    right clicks havve ID up from 0
    so in between 10000 clicks it won't be a problem.
    """
    
    ID = Point2D(10000,0)
    EventMode = avg.TOUCH
    pos1 = pos2 = (-1,-1)
    mouseDown1 = mouseDown2 = (-1,-1)
    hover = False  
    
    def __init__(self):
        """
        @param rootNode: this function is called with
        the rootnode of the application.
        """
        rootNode = Player.getRootNode()
        self.__container = Player.createNode('div', {})
        self.__p1 = Player.createNode('circle', {'r': 10, 'fillcolor':'ff0000',
            'fillopacity':0, 'opacity':0, 'sensitive':False})
        self.__p2 = Player.createNode('circle', {'r': 10, 'fillcolor':'ff0000',
            'fillopacity':0, 'opacity':0, 'sensitive':False})

        rootNode.appendChild(self.__container)
        self.__container.appendChild(self.__p1)
        self.__container.appendChild(self.__p2)
          
        rootNode.setEventHandler(avg.CURSORMOTION, avg.MOUSE, self.__onMouseMove)
        rootNode.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__mouseButtonDown)
        rootNode.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__mouseButtonUp)
            
    def delete(self): 
        """
        after turning off the emulater the function unlinks
        all nodes of the emulator. so it be garanted that the 
        application is working as before again.
        """
        rootNode = Player.getRootNode()

        rootNode.setEventHandler(avg.CURSORMOTION, avg.MOUSE, None)
        rootNode.setEventHandler(avg.CURSORDOWN, avg.MOUSE, None)
        rootNode.setEventHandler(avg.CURSORUP, avg.MOUSE, None)
        
        self.__container.unlink()
        self.__p1.unlink()
        self.__p2.unlink()
        
        self.__p1 = self.__p2 = self.__container = self.__node2root = None
        
    def _leftIDup(self):
        """
        ID for a left Click will be increased.
        """
        self.ID = self.ID + (1,0)
        
    def _getLeftID(self):
        """
        returns the current ID for left events (TOUCH or TRACK)
        """
        self.idpos1 = int(self.ID.x)
        return self.idpos1
        
    def _rightIDup(self):
        """
        ID for a right Click will be increased.
        """
        self.ID = self.ID + (0,1)
         
    def _getRightID(self):
        """
        returns the current ID for right events (TOUCH or TRACK)
        """
        self.idpos2 = int(self.ID.y)
        return self.idpos2
        
        
    def __mouseButtonDown(self,e):
        """
        activated if mousebutton is pressed (left or right press).
        a fake event will be created. 
        what kind of fake event (touch or track) depends on the EventMode. 
        by pressing 'ctrl left' the EventMode switches between TRACK
        and TOUCH.
        (default: TOUCH)
        
        when two events should be created it works better if you create first
        the left event and then the right event. so the handling of the two TOUCHES
        with only one mouse is better. 
        """ 
        
        if e.button == 1:
            self.pos1 = e.pos
            self.mouseDown1 = self.pos1
            self.__p1.fillopacity = 1
            Player.getTestHelper().fakeTouchEvent(self._getLeftID(), avg.CURSORDOWN,
                    self.EventMode, self.pos1, self.pos1, Point2D(0,0)) 
        elif e.button == 2:
            self.pos2 = e.pos - Point2D(50, 50)
            self.mouseDown2 = self.pos2
            self.__p2.fillopacity = 1
            Player.getTestHelper().fakeTouchEvent(self._getRightID(), avg.CURSORDOWN,
                    self.EventMode, self.pos2, self.pos2, Point2D(0,0))
        
        if self.EventMode == avg.TOUCH:
            self.drawTouch(e)
        else:
            self.drawTrack(e)
        return True
  
    def __onMouseMove(self, e):
        """
        is activated if one or two mouse buttons are pressed AND moved. 
        If the left and the right mouse button is pressed it creates 
        two fake events which are handled in the same way. what kind 
        of fake event (TOUCH/TRACK) depends on the EventMode. 
        by pressing 'ctrl left' the EventMode switches between TRACK
        and TOUCH.
        (default: TOUCH)
        
        left or right button pressed:
        TOUCH/TRACK will be created which gets the position of the mouse.
        
        left and right button pressed:
        two TRACK/TOUCH events will be created. the motion of the two events
        will be controlled by the mouse and a specific function.
        """
       
        if (e.leftbuttonstate and e.rightbuttonstate) or e.middlebuttonstate:
            self.pos1 = e.pos 
            self.pos2 = 2*e.lastdownpos - Point2D(50, 50) - e.pos
            Player.getTestHelper().fakeTouchEvent(self._getLeftID(), avg.CURSORMOTION,
                    self.EventMode, self.pos1, self.pos1, e.speed)
            Player.getTestHelper().fakeTouchEvent(self._getRightID(), avg.CURSORMOTION,
                    self.EventMode, self.pos2, self.pos2, e.speed)
        
        elif e.leftbuttonstate and not e.rightbuttonstate:
            self.pos1 = e.pos
            Player.getTestHelper().fakeTouchEvent(self._getLeftID(), avg.CURSORMOTION,
                    self.EventMode, self.pos1, self.pos1, e.speed)
            
        elif e.rightbuttonstate and not e.leftbuttonstate:
            self.pos2 = e.pos
            Player.getTestHelper().fakeTouchEvent(self._getRightID(), avg.CURSORMOTION,
                    self.EventMode, self.pos2, self.pos2, e.speed)
         
        if self.EventMode == avg.TOUCH:
            self.drawTouch(e)
        else:
            self.drawTrack(e)
        
        return True
       
       
       
    def __mouseButtonUp(self, e):
        """
        fake UP events will be created for the TOUCH or TRACK emulation.
        ID will be increased. 
        if event comes from left button:
        ID of a left event will be set to CURSORUP
        
        if event comes from right button:
        ID of a right event will be set to CURSORUP
        
        if event comes from both (button 3 = middlebutton)
        both actual events with their ID will be set to CURSORUP 
        """
        self.eraseDraw(e)
  
        if e.button == 3:
            Player.getTestHelper().fakeTouchEvent(self._getLeftID(), avg.CURSORUP,
                    self.EventMode, self.pos1, self.pos1, Point2D(0,0))
            Player.getTestHelper().fakeTouchEvent(self._getRightID(), avg.CURSORUP,
                    self.EventMode, self.pos2, self.pos2, Point2D(0,0))
            self.pos1 = (-1,-1)
            self.mouseDown1 = (-1,-1)
            self.pos2 = (-1,-1)
            self.mouseDown2 = (-1,-1)
            self._leftIDup()
            self._rightIDup()
        

        elif e.button == 1:
            Player.getTestHelper().fakeTouchEvent(self._getLeftID(), avg.CURSORUP,
                    self.EventMode, self.pos1, self.pos1, Point2D(0,0))
            self.pos1 = (-1,-1)
            self.mouseDown1 = (-1,-1)
            self._leftIDup()

        elif e.button == 2:
            Player.getTestHelper().fakeTouchEvent(self._getRightID(), avg.CURSORUP,
                    self.EventMode, self.pos2, self.pos2, Point2D(0,0))
            self.pos2 = (-1,-1)
            self.mouseDown2 = (-1,-1)
            self._rightIDup()
        
        return True
            
            
    def drawTouch(self,e):
        """
        function to draw the fake TOUCH events.
        
        color:    red (ff0000)
        radius:   6
        opacity:  1
        """
        self.__p1.pos = self.pos1
        self.__p1.r = 6
        self.__p1.fillcolor = 'ff0000'
        
        self.__p2.pos = self.pos2
        self.__p2.r = 6 
        self.__p2.fillcolor = 'ff0000'
        
    def eraseDraw(self,e):
        """
        function to erase the fake TOUCH events.
        opacity = 0
        """
        if e.button == 1:
            self.__p1.fillopacity = 0
        elif e.button == 2:
            self.__p2.fillopacity = 0
        elif e.button == 3:
            self.__p1.fillopacity = 0
            self.__p2.fillopacity = 0
    
    def drawTrack(self,e):
        """
        function to erase the fake TRACK events. opacity 
        and radius will be changed to get a hover similar 
        appearance.
        
        color:    white (ffffff)
        radius:   13
        opacity:  0.4
        
        """
        self.__p1.r = 13
        self.__p2.r = 13
        self.__p1.pos = self.pos1
        self.__p2.pos = self.pos2
        self.__p1.fillopacity = 0.4
        self.__p1.fillcolor = 'ffffff'
        self.__p2.fillopacity = 0.4
        self.__p2.fillcolor = 'ffffff'
    
    def changeMode(self):
        """
        function to switch between TRACK and TOUCH.
        it is called from the AVGAppStarter by pressing
        'ctrl left'.
        """ 
        self.hover = not self.hover
        Player.getTestHelper().fakeTouchEvent(self._getLeftID(),  avg.CURSORUP,
                self.EventMode, self.pos1, self.pos1, Point2D(0,0))
        Player.getTestHelper().fakeTouchEvent(self._getRightID(), avg.CURSORUP,
                self.EventMode, self.pos1, self.pos1, Point2D(0,0))
        if self.hover == True:
            self.EventMode = avg.TRACK
        else:
            self.EventMode = avg.TOUCH            

            
    
