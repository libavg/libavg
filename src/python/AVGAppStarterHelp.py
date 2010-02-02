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

        '?'     -->      on/off. 
"""

from libavg import avg

g_player = avg.Player.get()

class MThelp():

    def __init__(self, appStarter):
        self.__appstarter = appStarter
        self.__stackOfBackups = []      
        self.__stackKeyDown = []
        self.__stackKeyUp = []
        self.__stackUnicodeDown = []
        self.__stackUnicodeUp = []
        
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
        
        self.__keyBindDown = self.__appstarter.getKeys('key', 'down')
        self.__keyBindUp = self.__appstarter.getKeys('key', 'up')
        self.__keycodeBindDown = self.__appstarter.getKeys('unicode', 'down')
        self.__keycodeBindUp = self.__appstarter.getKeys('unicode', 'up')
        
        
        TextHelp = ''   
        if show == True:
            TextHelp = TextHelp + "<span><b>   ACTIVE KEYS </b><br/></span>"
            for key in sorted(self.__keyBindDown.iterkeys()):
                funcName = self.__keyBindDown[key][1]
                TextHelp = TextHelp + \
                    "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            for key in sorted(self.__keyBindUp.iterkeys()):
                if key in self.__keyBindDown:
                    print "hiereirieii"
                else:
                    funcName = self.__keyBindUp[key][1]
                    TextHelp = TextHelp + \
                        "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            for key in sorted(self.__keycodeBindDown.iterkeys()):
                funcName = self.__keycodeBindDown[key][1]
                TextHelp = TextHelp + \
                    "<span><b>%s</b>     <small>%s</small><br/></span>"% (key, funcName)
            
            for key in sorted(self.__keycodeBindUp.iterkeys()):
                if key in self.__keycodeBindDown:
                    pass
                else:
                    funcName = self.__keycodeBindUp[key][1]
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
        
        self.__stackKeyDown.append(self.__appstarter.getKeys('key', 'down'))
        self.__stackKeyUp.append(self.__appstarter.getKeys('key', 'up'))
        self.__stackUnicodeDown.append(self.__appstarter.getKeys('unicode','down'))
        self.__stackUnicodeUp.append(self.__appstarter.getKeys('unicode','up'))
        
        self.__appstarter.setKeys({}, 'key', 'down')
        self.__appstarter.setKeys({}, 'key', 'up')
        self.__appstarter.setKeys({}, 'unicode', 'down')
        self.__appstarter.setKeys({}, 'unicode', 'up')
        self.__appstarter.bindUnicode('?', self.__appstarter.activateHelp, 'HELP')
        
    def restoreKeys(self):
        # restore keybindings if last active app will be active again.
        # stored keys will be restored in the keybindings.
        
        self.__appstarter.showingHelp = False
        self.showHelp()  
        
        self.__appstarter.setKeys(self.__stackKeyDown.pop(), 'key', 'down')
        self.__appstarter.setKeys(self.__stackKeyUp.pop(), 'key', 'up')
        self.__appstarter.setKeys(self.__stackUnicodeDown.pop(), 'unicode', 'down')
        self.__appstarter.setKeys(self.__stackUnicodeUp.pop(), 'unicode', 'up')
        
