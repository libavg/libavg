#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, time, os, platform

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess.
sys.path += ['../wrapper/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg    # Under windows, there is no uninstalled version.
else:
    import avg

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)

from testcase import *

class DynamicsTestCase(AVGTestCase):

    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def __runDynamicsTest(self, createFunc, testName, isVideo = False, 
            warnOnImageDiff = False):

        def createNode1(useXml):
            
            def setNodeID():
                node.id = "bork"

            node = createFunc(useXml)
            node.id = "nodeid1"
            node.x = 10
            node.y = 20
            rootNode = Player.getRootNode()
            rootNode.appendChild(node)
            self.assertException(setNodeID)
            self.assert_(rootNode.indexOf(Player.getElementByID("nodeid1")) == 0)

        def createNode2(useXml):
            node = createFunc(useXml)
            node.id = "nodeid2"
            oldNode = Player.getElementByID("nodeid1")
            Player.getRootNode().insertChildBefore(node, oldNode)

        def reorderNode():
            Player.getRootNode().reorderChild(0, 1)
            node = Player.getElementByID("nodeid1")
            Player.getRootNode().reorderChild(node, 0)
        
        def removeNodes():
            self.node = Player.getElementByID("nodeid1")
            rootNode = Player.getRootNode()
            rootNode.removeChild(rootNode.indexOf(self.node))
            node2 = Player.getElementByID("nodeid2")
            rootNode.removeChild(node2)
            self.assert_(Player.getElementByID("nodeid1") == None)
        
        def reAddNode():
            rootNode = Player.getRootNode()
            rootNode.appendChild(self.node)
            if isVideo:
                self.node.play()
            self.node = None
        
        def removeAgain():
            node = Player.getElementByID("nodeid1")
            node.unlink()
            gone = Player.getElementByID("nodeid1")
            self.assert_(gone == None)
        
        def runTest(useXml):
            self._loadEmpty()
            createNode1(useXml)
            Player.stop()
            self.setUpVideo()
            self._loadEmpty()
            Player.setFakeFPS(25)
            self.start(None,
                    (lambda: createNode1(useXml),
                     lambda: self.compareImage(testName+"1", warnOnImageDiff),
                     lambda: createNode2(useXml),
                     lambda: self.compareImage(testName+"2", warnOnImageDiff),
                     reorderNode,
                     lambda: self.compareImage(testName+"3", warnOnImageDiff),
                     removeNodes,
                     lambda: self.compareImage(testName+"4", warnOnImageDiff),
                     reAddNode,
                     lambda: self.compareImage(testName+"5", warnOnImageDiff),
                     removeAgain
                    ))
        
        runTest(True)
        runTest(False) 

    def testImgDynamics(self):
    
        def createImg(useXml):
            if useXml:
                node = Player.createNode("<image href='rgb24-64x64.png'/>")
            else:
                node = Player.createNode("image", {"href":"rgb24-64x64.png"})
            return node

        self.__runDynamicsTest(createImg, "testImgDynamics")

    def testVideoDynamics(self):

        def createVideo(useXml):
            if useXml:
                node = Player.createNode(
                        "<video href='../video/testfiles/mpeg1-48x48.mpg'/>")
            else:
                node = Player.createNode("video", 
                        {"href":"../video/testfiles/mpeg1-48x48.mpg"})
            node.play()
            return node

        self.__runDynamicsTest(createVideo, "testVideoDynamics", True)

    def testWordsDynamics(self):

        def createWords(useXml):
            if useXml:
                node = Player.createNode("<words text='test'/>")
            else:
                node = Player.createNode("words", {"text":"test"})
            node.font="Bitstream Vera Sans"
            node.size=12
            node.parawidth=200
            return node

        self.__runDynamicsTest(createWords, "testWordsDynamics", False, True)

    def testPanoDynamics(self):
        def createPano(useXml):
            if useXml:
                node = Player.createNode("""
                    <panoimage href='panoimage.png' sensorwidth='4.60'
                            sensorheight='3.97' focallength='12'
                            width='160' height='120'/>
                    """)
            else:
                node = Player.createNode("panoimage", 
                    {"href":"panoimage.png", "sensorwidth":4.60, "sensorheight":3.97,
                     "focallength":12, "width":160, "height":120})
            return node

        self.__runDynamicsTest(createPano, "testPanoDynamics")

    def testDivDynamics(self):

        def createDiv(useXml):
            if useXml:
                node = Player.createNode("""
                    <div id='newDiv'>
                      <image href='rgb24-64x64.png'/>
                    </div>
                    """)
            else:
                imgNode = Player.createNode("image", {"href":"rgb24-64x64.png"})
                node = Player.createNode("div", {"id":"newDiv"})
                node.appendChild(imgNode)
            return node

        self.__runDynamicsTest(createDiv, "testDivDynamics")

    def testDynamicEventCapture(self):

        # Tests if deleting a node that has events captured works.
        def createImg():
            parentNode = Player.getRootNode()
            node = Player.createNode("image", {"id": "img", "href":"rgb24-64x64.png"})
            parentNode.appendChild(node)
            node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, captureMouseDown)
            parentNode.setEventHandler(avg.CURSORUP, avg.MOUSE, mainMouseUp)
        
        def setEventCapture():
            Player.getElementByID("img").setEventCapture()
        
        def deleteImg():
            parentNode = Player.getRootNode()
            node = Player.getElementByID("img")
            parentNode.removeChild(parentNode.indexOf(node))
        
        def captureMouseDown(event):
            self.captureMouseDownCalled = True
        
        def mainMouseUp(event):
            self.mainMouseUpCalled = True
        
        Helper = Player.getTestHelper()
        self.captureMouseDownCalled = False
        self.mainMouseUpCalled = False
        self._loadEmpty()
        self.start(None,
            (createImg,
            setEventCapture,
            lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 100, 10, 1),
            lambda: self.assert_(self.captureMouseDownCalled),
            deleteImg,
            lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False, 100, 10, 1),
            lambda: self.assert_(self.mainMouseUpCalled)
        ))



def dynamicsTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(DynamicsTestCase("testImgDynamics"))
    suite.addTest(DynamicsTestCase("testVideoDynamics"))
    suite.addTest(DynamicsTestCase("testWordsDynamics"))
#    suite.addTest(DynamicsTestCase("testPanoDynamics"))
    suite.addTest(DynamicsTestCase("testDivDynamics"))
    suite.addTest(DynamicsTestCase("testDynamicEventCapture"))
    return suite

Log = avg.Logger.get()
Log.setCategories(Log.APP |
        Log.WARNING
#         Log.PROFILE |
#         Log.PROFILE_LATEFRAMES |
#         Log.CONFIG |
#         Log.MEMORY |
#         Log.BLTS    |
#         Log.EVENTS |
#         Log.EVENTS2
              )

if os.getenv("AVG_CONSOLE_TEST"):
    sys.exit(0)
else:
    rmBrokenDir()
    Player = avg.Player()
    runner = unittest.TextTestRunner()
    rc = runner.run(dynamicsTestSuite())
    if rc.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)
        
