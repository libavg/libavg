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

class OffscreenTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)
    
    def testSceneBasics(self):
        def createScene(sceneName, x):
            Player.loadSceneString("""
                <?xml version="1.0"?>
                <scene id="%s" width="160" height="120">
                    <image id="test1" x="%s" href="rgb24-65x65.png" angle="0.4"/>
                </scene>
            """%(sceneName, str(x)))
            self.node = avg.ImageNode(parent=Player.getRootNode())
            self.node.href="scene:"+sceneName

        def changeHRef(href):
            self.node.href=href

        def setBitmap():
            bitmap = avg.Bitmap("rgb24-65x65.png")
            self.node.setBitmap(bitmap)

        def deleteScenes():
            Player.deleteScene("testscene1")
            self.assertException(lambda: changeHRef("scene:testscene1"))
            changeHRef("scene:testscene2")
            Player.deleteScene("testscene2")
            self.assertException(lambda: Player.deleteScene("foo"))

        self.loadEmptyScene()
        createScene("testscene1", 0)
        self.start(None, 
                (
                 lambda: self.compareImage("testOffscreen1", False),
                 self.node.unlink,
                 lambda: self.compareImage("testOffscreen2", False), 
                 lambda: Player.getRootNode().appendChild(self.node),
                 lambda: self.compareImage("testOffscreen1", False),
                 self.node.unlink,
                 lambda: createScene("testscene2", 80),
                 lambda: self.compareImage("testOffscreen3", False),
                 lambda: changeHRef("scene:testscene1"),
                 lambda: self.compareImage("testOffscreen1", False),
                 lambda: changeHRef("rgb24-65x65.png"),
                 lambda: self.compareImage("testOffscreen4", False),
                 lambda: changeHRef("scene:testscene1"),
                 lambda: self.compareImage("testOffscreen1", False),
                 setBitmap,
                 lambda: self.compareImage("testOffscreen4", False),
                 deleteScenes,
                 lambda: self.compareImage("testOffscreen3", False),
                ))

    def testSceneErrors(self):
        self.loadEmptyScene()
        # Missing id
        self.assertException(
                lambda: Player.loadSceneString("""<scene size="(160, 120)"/>"""))
        # Missing size
        self.assertException(
                lambda: Player.loadSceneString("""<scene id="foo"/>"""))
        # Duplicate scene id
        Player.loadSceneString("""<scene id="foo" size="(160, 120)"/>""")
        self.assertException(
                lambda: Player.loadSceneString("""<scene id="foo" size="(160, 120)"/>"""))


def offscreenTestSuite(tests):
    availableTests = (
            "testSceneBasics",
            "testSceneErrors"
            )
    return createAVGTestSuite(availableTests, OffscreenTestCase, tests)

Player = avg.Player.get()
