#!/usr/bin/python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

class DynamicsTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def __runDynamicsTest(self, createFunc, testName, isVideo = False, 
            warnOnImageDiff = False):

        def createNode1(useXml):
            
            def setNodeID():
                node.id = "bork"

            node = createFunc(useXml)
            node.id = "nodeid1"
            node.x = 10
            node.y = 20
            self.root.appendChild(node)
            self.assertRaises(RuntimeError, setNodeID)
            self.assertEqual(self.root.indexOf(player.getElementByID("nodeid1")), 0)
            self.assertRaises(RuntimeError, lambda: self.root.indexOf(self.root))

        def createNode2(useXml):
            node = createFunc(useXml)
            node.id = "nodeid2"
            oldNode = player.getElementByID("nodeid1")
            self.root.insertChildBefore(node, oldNode)

        def reorderNode():
            self.root.reorderChild(0, 1)
            node = player.getElementByID("nodeid1")
            self.root.reorderChild(node, 0)
        
        def removeNodes():
            self.node = player.getElementByID("nodeid1")
            self.root.removeChild(self.root.indexOf(self.node))
            node2 = player.getElementByID("nodeid2")
            self.root.removeChild(node2)
            self.assertEqual(player.getElementByID("nodeid1"), None)
        
        def reAddNode():
            self.root.appendChild(self.node)
            if isVideo:
                self.node.play()
            self.node = None
       
        def killNode():
            self.node = player.getElementByID("nodeid1")
            self.node.unlink(True)
            gone = player.getElementByID("nodeid1")
            self.assertEqual(gone, None)

        def removeAgain():
            node = player.getElementByID("nodeid1")
            node.unlink()
            gone = player.getElementByID("nodeid1")
            self.assertEqual(gone, None)
        
        def runTest(useXml):
            self.root = self.loadEmptyScene()
            createNode1(useXml)
            player.stop()
            self.root = self.loadEmptyScene()
            player.setFakeFPS(25)
            self.start(warnOnImageDiff, 
                    (lambda: createNode1(useXml),
                     lambda: self.compareImage(testName+"1"),
                     lambda: createNode2(useXml),
                     lambda: self.compareImage(testName+"2"),
                     reorderNode,
                     lambda: self.compareImage(testName+"3"),
                     removeNodes,
                     lambda: self.compareImage(testName+"4"),
                     reAddNode,
                     lambda: self.compareImage(testName+"5"),
                     killNode,
                     reAddNode,
                     removeAgain
                    ))
        
        runTest(True)
        runTest(False) 

    def testImgDynamics(self):
        def createImg(useXml):
            if useXml:
                node = player.createNode("<image href='rgb24-64x64.png'/>")
            else:
                node = player.createNode("image", {"href":"rgb24-64x64.png"})
            return node

        self.__runDynamicsTest(createImg, "testImgDynamics")

    def testVideoDynamics(self):
        def createVideo(useXml):
            if useXml:
                node = player.createNode(
                        "<video href='mpeg1-48x48.mov' threaded='false'/>")
            else:
                node = player.createNode("video", 
                        {"href":"mpeg1-48x48.mov", "threaded":False})
            node.play()
            return node

        self.__runDynamicsTest(createVideo, "testVideoDynamics", True)

    def testWordsDynamics(self):
        def createWords(useXml):
            if useXml:
                node = player.createNode("<words text='test'/>")
            else:
                node = player.createNode("words", {"text":"test"})
            node.font="Bitstream Vera Sans"
            node.fontsize=12
            node.width=200
            return node

        self.__runDynamicsTest(createWords, "testWordsDynamics", False, True)

    def testDivDynamics(self):
        def createDiv(useXml):
            if useXml:
                node = player.createNode("""
                    <div>
                      <image href='rgb24-64x64.png'/>
                    </div>
                    """)
            else:
                node = avg.DivNode()
                avg.ImageNode(href="rgb24-64x64.png", parent=node)
            return node

        self.__runDynamicsTest(createDiv, "testDivDynamics")

    def testDuplicateID(self):
        root = self.loadEmptyScene()
        avg.ImageNode(href="rgb24-64x64.png", id="testdup", parent=root)
        self.assertRaises(RuntimeError, lambda: avg.ImageNode(href="rgb24-64x64.png", 
                id="testdup", parent=root))
        self.start(False,
                (self.assertRaises(RuntimeError,
                        lambda: avg.ImageNode(href="rgb24-64x64.png", id="testdup",
                        parent=root)),
                ))
       
    def testChangeParentError(self):
        def changeParent():
            div = avg.DivNode()
            img = avg.ImageNode(href="additive/rgb24-64x64.png", parent=div)
            root.appendChild(img)

        root = self.loadEmptyScene()
        self.assertRaises(RuntimeError, changeParent)
        self.start(False, (self.assertRaises(RuntimeError, changeParent),))

    def testDynamicEventCapture(self):
        # Tests if deleting a node that has events captured works.
        def createImg():
            parentNode = root
            node = player.createNode("image", {"id": "img", "href":"rgb24-64x64.png"})
            parentNode.appendChild(node)
            node.subscribe(avg.Node.CURSOR_DOWN, captureMouseDown)
            parentNode.subscribe(avg.Node.CURSOR_UP, mainMouseUp)
        
        def setEventCapture():
            player.getElementByID("img").setEventCapture()
        
        def deleteImg():
            parentNode = root
            node = player.getElementByID("img")
            parentNode.removeChild(parentNode.indexOf(node))
        
        def captureMouseDown(event):
            self.captureMouseDownCalled = True
        
        def mainMouseUp(event):
            self.mainMouseUpCalled = True
        
        self.captureMouseDownCalled = False
        self.mainMouseUpCalled = False
        root = self.loadEmptyScene()
        self.start(False,
                (createImg,
                 setEventCapture,
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_DOWN, 100, 10),
                 lambda: self.assert_(self.captureMouseDownCalled),
                 deleteImg,
                 lambda: self._sendMouseEvent(avg.Event.CURSOR_UP, 100, 10),
                 lambda: self.assert_(self.mainMouseUpCalled)
                ))

    def testEventBubbling(self):
        def click (x, y):
            self.fakeClick(x, y)

        def createNodes():
            def appendEventString (s):
                self.__eventString += s
                return True

            def setHandler (node, s, swallow = False):
                node.setEventHandler(avg.Event.CURSOR_DOWN, avg.Event.MOUSE, 
                        lambda e: appendEventString(s) and swallow)

            parentNode = root
            node = player.createNode("div", {'x':0,'y':0,'width':50, 'height':50})
            setHandler (node, 'a')
            parentNode.appendChild(node)
            node = player.createNode("div", {'x':0,'y':0,'width':100, 'height':100})
            setHandler (node, 'b')
            parentNode.insertChild(node,0)
            parentNode = node
            node = player.createNode("div", {'x':40,'y':40,'width':30, 'height':30})
            setHandler (node, 'c')
            parentNode.appendChild(node)
            node = player.createNode("div", {'x':60,'y':40,'width':30, 'height':30})
            setHandler (node, 'd', True)
            parentNode.appendChild(node)

        def resetEventString():
            self.__eventString = ''

        root = self.loadEmptyScene()
        self.start(False,
                (createNodes,
                 resetEventString,
                 lambda: click (10,10),
                 lambda: self.assertEqual(self.__eventString, 'a'),
                 resetEventString,
                 lambda: click (55,55),
                 lambda: self.assertEqual(self.__eventString, 'cb'),
                 resetEventString,
                 lambda: click (65,55),
                 lambda: self.assertEqual(self.__eventString, 'd'),
                ))

    def testComplexDiv(self):
        def setImageID(imgNode):
            imgNode.id = "imageid"

        def createDiv():
            imgNode = player.createNode("image", 
                    {"id":"imageid", "href":"rgb24-64x64.png"})
            node = player.createNode("div", {"id":"divid"})
            node.appendChild(imgNode)
            imgNode.id = "imageid"
            root.appendChild(node)
            self.assertRaises(RuntimeError, lambda: setImageID(imgNode))
  
        def removeDiv():
            node = player.getElementByID("divid")
            imgNode = player.getElementByID("imageid")
            node.unlink()
            imgNode.id = "imageid"
            imgNode.unlink()
            root.appendChild(node)
            node.appendChild(imgNode)
            self.assertRaises(RuntimeError, lambda: setImageID(imgNode))

        root = self.loadEmptyScene()
        createDiv()
        removeDiv()
        player.stop()
        root = self.loadEmptyScene()
        player.setFakeFPS(25)
        self.start(False,
                (createDiv,
                 lambda: self.compareImage("testComplexDiv1"),
                 removeDiv,
                 lambda: self.compareImage("testComplexDiv1"),
                ))

    def testNodeCustomization(self):
        def testNodePythonAttribute():
            node1 = player.createNode("image", {"id":"foo", "pos":(23, 42)})
            root.appendChild(node1)
            node1.customAttribute = "bbb"
            node2 = player.getElementByID("foo")
            self.assertEqual(node1, node2)
            self.assertEqual(node2.customAttribute, "bbb")
            node1.unlink(True)

        def testNodePythonSubclass():

            class CustomImageNode(avg.ImageNode):
                def __init__(self, p, parent=None, **kwargs):
                    avg.ImageNode.__init__(self, pos=p, href="rgb24-64x64.png", **kwargs)
                    self.registerInstance(self, parent)

                def customMethod(self):
                    pass

            class CustomDivNode(avg.DivNode):
                def __init__(self, parent=None, **kwargs):
                    avg.DivNode.__init__(self, **kwargs)
                    self.registerInstance(self, parent)
                    CustomImageNode((23,42), parent=self)


            customNode = CustomImageNode((23, 42), parent=root)
            retrievedImage = root.getChild(0)
            self.assertEqual(type(retrievedImage), CustomImageNode)
            self.assertEqual(retrievedImage.pos, (23,42))
            self.assertEqual(retrievedImage.href, "rgb24-64x64.png")
            retrievedImage.customMethod()
            customNode.unlink(True)
            
            CustomDivNode(parent=player.getRootNode())
            retrievedDiv = player.getRootNode().getChild(0)
            self.assertEqual(type(retrievedDiv), CustomDivNode)
            retrievedImage = retrievedDiv.getChild(0)
            self.assertEqual(type(retrievedImage), CustomImageNode)
            retrievedDiv = retrievedImage.parent
            self.assertEqual(type(retrievedDiv), CustomDivNode)
            retrievedDiv.unlink(True)
            
            customNode = CustomImageNode((23,42))
            root.appendChild(customNode)
            retrievedImage = root.getChild(0)
            self.assertEqual(type(retrievedImage), CustomImageNode)

        root = self.loadEmptyScene()
        testNodePythonAttribute()
        testNodePythonSubclass()

    def testDynamicMediaDir(self):
        def attachNode():
            root.appendChild(imageNode1)

        root = self.loadEmptyScene()
        root.mediadir="testmediadir"
        imageNode1 = player.createNode("image", {"href": "rgb24-64x64a.png"})
        imageNode2 = player.createNode("image", {"href": "rgb24-64x64a.png", "x":30})
        root.appendChild(imageNode2)
        self.start(False,
                (lambda: self.compareImage("testDynamicMediaDir1"),
                 attachNode,
                 lambda: self.compareImage("testDynamicMediaDir2")
                ))


def dynamicsTestSuite(tests):
    availableTests = (
            "testImgDynamics",
            "testVideoDynamics",
            "testWordsDynamics",
            "testDivDynamics",
            "testEventBubbling",
            "testDuplicateID",
            "testChangeParentError",
            "testDynamicEventCapture",
            "testComplexDiv",
            "testNodeCustomization",
            "testDynamicMediaDir",
            )

    return createAVGTestSuite(availableTests, DynamicsTestCase, tests)
