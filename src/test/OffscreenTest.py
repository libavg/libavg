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
            scene = self.__createOffscreenScene(sceneName, False)
            scene.getElementByID("test1").x = x
            self.node = avg.ImageNode(parent=Player.getRootNode())
            self.node.href="scene:"+sceneName

        def changeHRef(href):
            self.node.href = href

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
                (lambda: self.compareImage("testOffscreen1", False),
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

    def testSceneLoadAfterPlay(self):
        def __createOffscreenScene():
            offscreenScene = self.__createOffscreenScene("offscreenscene", False)
            self.node = avg.ImageNode(parent=Player.getRootNode(), 
                    href="scene:offscreenscene")
    
        self.loadEmptyScene()
        self.start(None,
                (
                 __createOffscreenScene,
                 lambda: self.compareImage("testOffscreen1", False),
                ))

    def testSceneResize(self):
        def setSize():
            self.node.size = (80, 60)

        mainScene, offscreenScene = self.__setupScene(False)
        self.start(None,
                (setSize,
                 lambda: self.compareImage("testSceneResize", False)
                ))

    def testSceneErrors(self):
        self.loadEmptyScene()
        # Missing size
        self.assertException(
                lambda: Player.loadSceneString("""<scene id="foo"/>"""))
        # Duplicate scene id
        Player.loadSceneString("""<scene id="foo" size="(160, 120)"/>""")
        self.assertException(
                lambda: Player.loadSceneString("""<scene id="foo" size="(160, 120)"/>"""))

    def testSceneAPI(self):
        def checkMainScreenshot():
            bmp1 = Player.screenshot()
            bmp2 = mainScene.screenshot()
            self.assert_(self.areSimilarBmps(bmp1, bmp2, 0.01, 0.01))

        def checkSceneScreenshot():
            bmp = offscreenScene.screenshot()
            self.compareBitmapToFile(bmp, "testOffscreenScreenshot", False)

        mainScene = self.loadEmptyScene()
        self.assert_(mainScene == Player.getMainScene())
        self.assert_(mainScene.getRootNode() == Player.getRootNode())
        offscreenScene = self.__createOffscreenScene("offscreenscene", False)
        self.assert_(offscreenScene == Player.getScene("offscreenscene"))
        self.assert_(offscreenScene.getElementByID("test1").href == "rgb24-65x65.png")
        self.assert_(offscreenScene.getElementByID("missingnode") == None)
        self.assertException(Player.screenshot())
        self.start(None, 
                (checkMainScreenshot,
                 checkSceneScreenshot))

    def testSceneEvents(self):
        def onOffscreenImageDown(event):
            self.__offscreenImageDownCalled = True

        def onMainDown(event):
            self.__mainDownCalled = True

        def reset():
            self.__offscreenImageDownCalled = False
            self.__mainDownCalled = False

        def setPos():
            self.node.pos = (80, 60)
            self.node.size = (80, 60)

        mainScene, offscreenScene = self.__setupScene(True)
        offscreenImage = offscreenScene.getElementByID("test1")
        offscreenImage.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onOffscreenImageDown)
        Player.getRootNode().setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMainDown)
        helper = Player.getTestHelper()
        self.__offscreenImageDownCalled = False
        self.__mainDownCalled = False
        self.start(None,
                (lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        10, 10, 1),
                 lambda: self.assert_(self.__offscreenImageDownCalled),
                 reset,
                 lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        80, 10, 1),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled)),
                 reset,
                 setPos,
                 lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        70, 65, 1),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled)),
                 lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        120, 65, 1),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled)),
                 reset,
                 lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        110, 65, 1),
                 lambda: self.assert_(self.__offscreenImageDownCalled and 
                        not(self.__mainDownCalled)),
                 reset,
                 lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        1, 1, 1),
                 lambda: self.assert_(not(self.__offscreenImageDownCalled) and 
                        self.__mainDownCalled),
                ))

    def testSceneEventCapture(self):
        def onOffscreenImageDown(event):
            self.__offscreenImageDownCalled = True

        mainScene, offscreenScene = self.__setupScene(True)
        offscreenImage = offscreenScene.getElementByID("test1")
        offscreenImage.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onOffscreenImageDown);
        helper = Player.getTestHelper()
        self.__offscreenImageDownCalled = False
        offscreenImage.setEventCapture()
        self.start(None,
                (lambda: helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 
                        80, 10, 1),
                 lambda: self.assert_(self.__offscreenImageDownCalled),
                ))
                
    def testSceneRender(self):
        def renderScene():
            scene = self.__createOffscreenScene("testscene", False)
            scene.render()
            bmp = scene.screenshot()
            self.compareBitmapToFile(bmp, "testOffscreenScreenshot", False)
            Player.deleteScene("testscene")
            scene = None

        mainScene = self.loadEmptyScene()
        self.assertException(renderScene);
        Player.deleteScene("testscene")
        self.start(None,
                (renderScene,
                ))
        
    def testSceneCrop(self):
        mainScene = self.loadEmptyScene()
        scene = Player.loadSceneString("""
            <?xml version="1.0"?>
            <scene id="testscene" width="160" height="120">
                <div pos="(40, 30)" size="(80, 60)">
                    <image id="test1" pos="(-32, -32)" href="rgb24-65x65.png"/>
                </div>
            </scene>
        """)
        node = avg.ImageNode(parent=Player.getRootNode(), 
                href="scene:testscene")
        self.start(None,
                (lambda: self.compareImage("testSceneCrop", False),
                ))

    def testSceneAlpha(self):
        mainScene = self.loadEmptyScene()
        scene = Player.loadSceneString("""
            <?xml version="1.0"?>
            <scene id="testscene" width="80" height="120">
                <image id="test1" href="rgb24alpha-64x64.png"/>
            </scene>
        """)
        avg.RectNode(parent=Player.getRootNode(), fillcolor="FFFFFF",
                pos=(0.5, 0.5), size=(160, 48), fillopacity=1)
        node = avg.ImageNode(parent=Player.getRootNode(), 
                href="scene:testscene")
        avg.ImageNode(parent=Player.getRootNode(), x=64, href="rgb24alpha-64x64.png")
        self.start(None,
                (lambda: self.compareImage("testSceneAlpha", False),
                ))


    def testSceneMultisampling(self):
        mainScene = self.loadEmptyScene()
        scene = Player.loadSceneString("""
            <?xml version="1.0"?>
            <scene id="testscene" width="160" height="120" multisamplesamples="2">
                <image id="test1" href="rgb24-65x65.png" angle="0.1"/>
            </scene>
        """)
        self.assert_(scene.multisamplesamples == 2)
        node = avg.ImageNode(parent=Player.getRootNode(), 
                href="scene:testscene")
        self.start(None,
                (lambda: self.compareImage("testSceneMultisample", False),
                ))
        

    def __setupScene(self, handleEvents):
        mainScene = self.loadEmptyScene()
        offscreenScene = self.__createOffscreenScene("offscreenscene", handleEvents)
        self.node = avg.ImageNode(parent=Player.getRootNode(), 
                href="scene:offscreenscene")
        return (mainScene, offscreenScene)

    def __createOffscreenScene(self, sceneName, handleEvents):
        return Player.loadSceneString("""
            <?xml version="1.0"?>
            <scene id="%s" width="160" height="120" handleevents="%s">
                <image id="test1" href="rgb24-65x65.png"/>
            </scene>
        """%(sceneName, str(handleEvents)))


def offscreenTestSuite(tests):
    availableTests = (
            "testSceneBasics",
            "testSceneLoadAfterPlay",
            "testSceneResize",
            "testSceneErrors",
            "testSceneAPI",
            "testSceneEvents",
            "testSceneEventCapture",
            "testSceneRender",
            "testSceneCrop",
            "testSceneAlpha",
            "testSceneMultisampling",
            )
    return createAVGTestSuite(availableTests, OffscreenTestCase, tests)


Player = avg.Player.get()
