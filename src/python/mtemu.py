#!/usr/bin/env python
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
# Original author of this file is Sebastian Maulbeck 
# <sm (at) archimedes-solutions (dot) de>


"""
this class provides a test to emulate one or two TOUCH/TRACK events. 
by pressing "ctrl left/right" the TOUCH events will be switched into TRACK events and the 
other way around.
by pressing "shift left/right" a second event is created whenever the mousebutton (left) 
is clicked. 
the second event appears at the position:   mousePosition - (40,40) pixels.

Note: remove any mouse event handling from your application to avoid emulation issues

For example:

1) avg.MOUSE | avg.TOUCH filters can be safely reduced to avg.TOUCH
2) the current libavg button module breaks the emulation
3) the current libavg Grabbable class breaks the emulation, unless is created with 
   source = avg.TOUCH parameter
"""

from libavg import avg, Point2D
import warnings

Player = avg.Player.get()

class MTemu(object): 
    """
    this class provides a test method to
    emulate one or two TOUCH events. 
    by pressing "ctrl left" the TOUCH events 
    will be switched into TRACK events.
    """
    
    ID = Point2D(10000,0)
    EventMode = avg.TOUCH
    pos1 = pos2 = Point2D(-1,-1)
    mouseDown1 = mouseDown2 = Point2D(-1,-1)
    hover = False  
    multiActive = False
    mouseState = ''
    
    def __init__(self):
        rootNode = Player.getRootNode()
        posX = rootNode.size.x * 3/4
        posY = rootNode.size.y-40
        
        self.__layer = Player.createNode(''' 
                <words id="displayEmu" x="%(posX)i" y="%(posY)i" fontsize="20" opacity="1" 
                color="DDDDDD" text="multitouch emulation active!" sensitive="False" />
                '''
                % {'posX':posX, 'posY':posY} 
                )
        rootNode.appendChild(self.__layer)  
                   
        self.__container = Player.createNode('div', {})
        self.__p1 = Player.createNode('circle', {'r': 10, 'fillcolor':'ff0000',
            'fillopacity':0, 'opacity':0, 'sensitive':False})
        self.__p2 = Player.createNode('circle', {'r': 10, 'fillcolor':'ff0000',
            'fillopacity':0, 'opacity':0, 'sensitive':False})

        rootNode.appendChild(self.__container)
        self.__container.appendChild(self.__p1)
        self.__container.appendChild(self.__p2)
          
        rootNode.setEventHandler(avg.CURSORMOTION, avg.MOUSE, self.__onMouseMove)
        rootNode.setEventHandler(avg.CURSORDOWN, avg.MOUSE, self.__onMouseButtonDown)
        rootNode.setEventHandler(avg.CURSORUP, avg.MOUSE, self.__onMouseButtonUp)
        
                    
    def delete(self): 
        """
        after turning off the emulater the function unlinks
        all nodes of the emulator. so it be garanted that the 
        application is working as before again.
        events will be cleared with two UP events.
        """
        rootNode = Player.getRootNode()
        rootNode.setEventHandler(avg.CURSORMOTION, avg.MOUSE, None)
        rootNode.setEventHandler(avg.CURSORDOWN, avg.MOUSE, None)
        rootNode.setEventHandler(avg.CURSORUP, avg.MOUSE, None)
        self.__layer.unlink()
        self.__container.unlink()
        self.__p1.unlink()
        self.__p2.unlink()
        self.__p1 = self.__p2 = None
        self.__container = self.__node2root = self.__layer = None
        Player.getTestHelper().fakeTouchEvent(self.__getLeftID(), avg.CURSORUP,
                    self.EventMode, self.pos1, self.pos1, Point2D(0,0))
        Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORUP,
                    self.EventMode, self.pos2, self.pos2, Point2D(0,0))
        
    def __leftIDup(self):
        self.ID = self.ID + (1,0)
        
    def __getLeftID(self):
        self.idpos1 = int(self.ID.x)
        return self.idpos1
        
    def __rightIDup(self):
        self.ID = self.ID + (0,1)
         
    def __getRightID(self):
        self.idpos2 = int(self.ID.y)
        return self.idpos2
        
        
    def __onMouseButtonDown(self,e):
        #activation with a left mouse click.
        #one or two fake UP events will be created (multitouch active or not). 
        #SOURCE depending on EventMode. 
        
        if e.button == 1:
            self.mouseState = 'Down'
            self.pos1 = e.pos
            self.mouseDown1 = self.pos1
            self.__leftIDup()
            Player.getTestHelper().fakeTouchEvent(self.__getLeftID(), avg.CURSORDOWN,
                        self.EventMode, self.pos1, self.pos1, e.speed)
            
            if self.multiActive:
                
                self.mouseDown2 = e.pos - Point2D(20,20)
                self.pos2 = 2*Point2D(self.mouseDown2) - Point2D(20,20) - e.pos
                self.__rightIDup()
                Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORDOWN,
                        self.EventMode, self.pos2, self.pos2, -e.speed)
                            
            self.__drawTouch()
        return True

    def __onMouseMove(self, e):
        #is activated if left mouse button is pressed AND moved. 
        
        #If the left mouse button is pressed and multitouch (shift) 
        #is acitvated it creates two fake events. 
        #One event position will be set to the mouse position. 
        #the second event will move in relation to the mouse position. 
        
        if e.leftbuttonstate and self.multiActive:
            self.pos1 = e.pos 
            self.pos2 = 2*Point2D(self.mouseDown2) - Point2D(20,20) - e.pos
            Player.getTestHelper().fakeTouchEvent(self.__getLeftID(), avg.CURSORMOTION,
                    self.EventMode, self.pos1, self.pos1, e.speed)
            Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORMOTION,
                    self.EventMode, self.pos2, self.pos2, -e.speed)
            self.__drawTouch()
            
        elif e.leftbuttonstate and not self.multiActive:
            self.pos1 = e.pos
            Player.getTestHelper().fakeTouchEvent(self.__getLeftID(), avg.CURSORMOTION,
                    self.EventMode, self.pos1, self.pos1, e.speed)
            self.__drawTouch()
         
        return True
       
       
    def __onMouseButtonUp(self, e):
        #one or two fake UP events will be created (multitouch active or not). 
        #SOURCE depending on EventMode. 
            
        self.__eraseDraw()  
        if e.button == 1:
            self.mouseState = 'Up'
            Player.getTestHelper().fakeTouchEvent(self.__getLeftID(), avg.CURSORUP,
                    self.EventMode, self.pos1, self.pos1, e.speed)
            self.pos1 = (-1,-1)
            self.mouseDown1 = (-1,-1)

            
            if self.multiActive:
                Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORUP,
                        self.EventMode, self.pos2, self.pos2, -e.speed)
                self.pos2 = (-1,-1)
                self.mouseDown2 = (-1,-1)

        return True
            
            
    def __drawTouch(self):
        #function to draw the fake events. style depending on EventMode.
        if self.EventMode == avg.TOUCH:
            self.__p1.pos = self.pos1
            self.__p1.r = 6
            self.__p1.fillcolor = 'ff0000'
            self.__p1.fillopacity = 1
            
            self.__p2.pos = self.pos2
            self.__p2.r = 6 
            self.__p2.fillcolor = 'ff0000'
            if self.multiActive:
                self.__p2.fillopacity = 1
            else:
                self.__p2.fillopacity = 0
        else:
            self.__p1.r = 13
            self.__p1.pos = self.pos1
            self.__p1.fillcolor = 'ffffff'
            self.__p1.fillopacity = 0.4
            
            self.__p2.r = 13
            self.__p2.pos = self.pos2
            self.__p2.fillcolor = 'ffffff'
            if self.multiActive:
                self.__p2.fillopacity = 0.4
            else:
                self.__p2.fillopacity = 0
        
        
    def __eraseDraw(self):
        #function to erase the fake TOUCH events.
        
        self.__p1.fillopacity = 0
        if self.multiActive:
            self.__p2.fillopacity = 0

    def changeMode(self):
        """
        function to switch between TRACK and TOUCH.
        it is called from the AVGAppStarter by pressing
        'ctrl left'.
        """ 
        
        self.__eraseDraw()
        Player.getTestHelper().fakeTouchEvent(self.__getLeftID(),  avg.CURSORUP,
                self.EventMode, self.pos1, self.pos1, Point2D(0,0))

        if self.multiActive:
            Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORUP,
                    self.EventMode, self.pos2, self.pos2, Point2D(0,0))
   
        self.hover = not self.hover
        if self.hover == True:
            self.EventMode = avg.TRACK
        else:
            self.EventMode = avg.TOUCH            
            
        if self.mouseState == 'Down':     
            self.mouseDown1 = self.pos1  
            self.__leftIDup()
            Player.getTestHelper().fakeTouchEvent(self.__getLeftID(),  avg.CURSORDOWN,
                    self.EventMode, self.pos1, self.pos1, Point2D(0,0))
            if self.multiActive:
                e = Player.getMouseState()
                self.mouseDown2 = e.pos - Point2D(20,20)
                self.pos2 = 2*Point2D(self.mouseDown2) - Point2D(20,20) - e.pos
                self.__rightIDup()
                Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORDOWN,
                        self.EventMode, self.pos2, self.pos2, Point2D(0,0))
            self.__drawTouch()
        
    def multiTouch(self):
        """
        creates another event if shift and mouse left is pressed.
        if the mouse is not pressed no event will appear.
        """
        self.multiActive = not self.multiActive  
         
        if self.mouseState == 'Down': 
            self.__eraseDraw()
            
            if not self.multiActive:
                Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORUP,
                        self.EventMode, self.pos2, self.pos2, Point2D(0,0))
            
            if self.multiActive:
                e = Player.getMouseState()
                self.mouseDown2 = e.pos - Point2D(20,20)
                self.pos2 = 2*Point2D(self.mouseDown2) - Point2D(20,20) - e.pos 
                self.__rightIDup() 
                Player.getTestHelper().fakeTouchEvent(self.__getRightID(), avg.CURSORDOWN,
                        self.EventMode, self.pos2, self.pos2, Point2D(0,0))
            
            self.__drawTouch()
            
        return True
        
