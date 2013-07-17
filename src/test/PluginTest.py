#!/usr/bin/python
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

import platform

from libavg import player
from testcase import *

class PluginTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def testColorNodePlugin(self):
        def loadPlugin():
            if platform.system() != 'Windows':
                if not(os.getenv('srcdir') in ('.', None)):
                    # make distcheck
                    player.pluginPath += ":../../_build/src/test/plugin/.libs"
            player.loadPlugin("colorplugin")
            
        def usePlugin1():
            node = colorplugin.ColorNode(fillcolor="7f7f00", id="mynode1")
            root.appendChild(node)
            
            mynode = player.getElementByID("mynode1")
            self.assertEqual(mynode.fillcolor, "7f7f00")
 
        def usePlugin2():
            node = player.createNode('<colornode fillcolor="0f3f7f" id="mynode2" />')
            root.appendChild(node)

            mynode = player.getElementByID("mynode2")
            self.assertEqual(mynode.fillcolor, "0f3f7f")

        root = self.loadEmptyScene()
        self.start(False,
                (loadPlugin,
                 usePlugin1,
                 lambda: self.compareImage("testplugin1"),
                 usePlugin2,
                 lambda: self.compareImage("testplugin2"),
                ))

def pluginTestSuite (tests):
    availableTests = ("testColorNodePlugin",)
    return createAVGTestSuite(availableTests, PluginTestCase, tests)
