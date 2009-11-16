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
this class is for displaying all active keys. 

        '#'     -->      on/off. 
"""

from libavg import avg

g_player = avg.Player.get()

class MThelp():

    def __init__(self, appStarter):
        self.__appstarter = appStarter
        self.__stackOfBackups = []      
        self.__rectNode = g_player.createNode("""
            <div id="RectNode" opacity="0" sensitive="False" x="5" y="10" size="(450,450)"> 
                <rect id="rectWhite" strokewidth="0" fillopacity="0.1" 
                        fillcolor="FFFFFF" pos="(5,5)" size="(450,450)"/>
                <rect id="rectBlack" strokewidth="0" fillopacity="0.6" 
                        fillcolor="000000" pos="(6,6)" size="(451,451)"/>
            </div>
        """)
        self.__keysNode = g_player.createNode("""
                    <words x="10" y="10" fontsize="18" opacity="1" 
                            color="DDDDDD" text=""/>
            """)
        rootNode = g_player.getRootNode()
        rootNode.appendChild(self.__rectNode)
        self.__rectNode.appendChild(self.__keysNode)
       
    def showHelp(self):
        show = self.__appstarter.showingHelp
        self.__keyBindDown = self.__appstarter.getKeyDowns()
        TextHelp = ''   
        if show == True:
            for key in sorted(self.__keyBindDown.iterkeys()):
                funcName = self.__keyBindDown[key][1]
                TextHelp = TextHelp + \
                    "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            self.__keysNode.text = TextHelp
            self.__rectNode.opacity = 1
            g_player.getElementByID('rectWhite').size = self.__keysNode.getMediaSize()
            g_player.getElementByID('rectBlack').size = self.__keysNode.getMediaSize()
            
            self.__rectNode.getParent().reorderChild(
                    self.__rectNode.getParent().indexOf(self.__rectNode), 
                    self.__rectNode.getParent().getNumChildren()-1) 
            
        elif show == False:
            self.__keysNode.text = ''
            self.__rectNode.opacity = 0
            
    def backupKeys(self):
        # backup keybindings if you change active app.
        # keys will be stored in a dict. keybindings from new function can be loaded
        # and conflicts by double keybindings are reduced to only the active app.
        
        self.__appstarter.showingHelp = False
        self.showHelp()
        self.__keyBindDown = self.__appstarter.getKeyDowns()
        self.__keyBindUp = self.__appstarter.getKeyUps()
        
        self.__stackOfBackups.append(self.__keyBindDown)
        self.__stackOfBackups.append(self.__keyBindUp)
        self.__appstarter.setKeyDowns({})
        self.__appstarter.setKeyUps({}) 
        self.__appstarter.bindKey('l', self.__appstarter.activateHelp, 'HELP')
      
    def restoreKeys(self):
        # restore keybindings if last active app will be active again.
        # stored keys will be restored in the keybindings.
        
        self.__appstarter.showingHelp = False
        self.showHelp()  
        self.__keyBindUp = self.__appstarter.setKeyUps(self.__stackOfBackups.pop())
        self.__keyBindDown = self.__appstarter.setKeyDowns(self.__stackOfBackups.pop())
