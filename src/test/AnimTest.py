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

class AnimTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def initScene(self):
        root = self.loadEmptyScene()
        self.__node = avg.ImageNode(id="test", pos=(64,30), href="rgb24-65x65.png",
                parent=root)
        player.setFakeFPS(10)

    def testAnimType(self, curAnim, imgBaseName):
        def onStop():
            self.__onStopCalled = True

        def startAnim():
            self.__onStopCalled = False
            self.__anim.start()

        def startKeepAttr():
            self.__node.x = 32 
            self.__anim.start(True)

        def abortAnim():
            self.__anim.abort()

        self.__anim = curAnim
        self.__anim.setStopCallback(onStop)
        self.__onStopCalled = False
        self.assertRaises(RuntimeError, lambda: self.__anim.start())
        self.start(False,
                (startAnim,
                 lambda: self.compareImage(imgBaseName+"1"),
                 lambda: self.compareImage(imgBaseName+"2"),
                 lambda: self.compareImage(imgBaseName+"3"),
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assert_(not(self.__anim.isRunning())),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 lambda: self.compareImage(imgBaseName+"4"),
                 lambda: self.assertEqual(self.__node.x, 100),
                 startAnim,
                 lambda: self.compareImage(imgBaseName+"1"),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 1),
                 abortAnim,
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 lambda: self.compareImage(imgBaseName+"5"),
                 lambda: self.assert_(not(self.__anim.isRunning())),
                 None,
                 lambda: self.assert_(self.__onStopCalled),
                 startKeepAttr,
                 lambda: self.compareImage(imgBaseName+"6"),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 1)
                ))
        self.__anim = None

    def testLinearAnim(self):
        self.initScene()
        curAnim = avg.LinearAnim(self.__node, "x", 300, 0, 100)
        self.testAnimType(curAnim, "testLinearAnimC")

    def testAnimRegistry(self):
        def on1Stop():
            self.__onStopCalled = True
        
        def on2Start():
            self.__onStopBeforeOnStart = self.__onStopCalled
        
        self.initScene()
        sameNode = player.getElementByID("test")
        anim1 = avg.LinearAnim(self.__node, "x", 500, 0, 100,
                               False, None, on1Stop)
        anim2 = avg.LinearAnim(sameNode, "x", 300, 0, 100,
                               False, on2Start)
        self.__onStopCalled = False
        self.__onStopBeforeOnStart = False
        self.start(False,
                (lambda: anim1.start(),
                 lambda: self.assert_(not(self.__onStopCalled)),
                 lambda: anim2.start(),
                 lambda: self.assert_(self.__onStopBeforeOnStart),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 1)
                ))
        anim1 = None
        anim2 = None

    def testFadeIn(self):
        def onStop():
            self.__onStopCalled = True

        self.initScene()
        self.__node.opacity=0.5
        self.__onStopCalled = False
        self.start(False,
                (lambda: avg.fadeIn(self.__node, 200, 1, onStop),
                 lambda: self.compareImage("testFadeIn1"),
                 lambda: self.compareImage("testFadeIn2"),
                 lambda: self.compareImage("testFadeIn3"),
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0)
                ))
        self.__anim = None

    def testFadeOut(self):
        def onStop():
            self.__onStopCalled = True

        self.initScene()
        self.__node.opacity=0.5
        self.__onStopCalled = False
        self.start(False,
                (lambda: avg.fadeOut(self.__node, 200, onStop),
                 lambda: self.compareImage("testFadeOut1"),
                 lambda: self.compareImage("testFadeOut2"),
                 lambda: self.compareImage("testFadeOut3"),
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0)
                ))
        self.__anim = None

    def testNonExistentAttributeAnim(self):
        self.initScene()
        self.assertRaises(Exception,
                lambda: avg.LinearAnim(self.__node, "foo", 0, 0, 0, False))
        self.assertRaises(Exception, lambda: avg.LinearAnim(None, "x", 0, 0, 0, False))

    def testLinearAnimZeroDuration(self):
        def onStop():
            self.__onStopCalled = True

        def startAnim():
            self.__onStopCalled = False
            self.__anim.start()

        self.initScene()
        self.__anim = avg.LinearAnim(self.__node, "x", 0, 0, 100, False, None, onStop)
        self.__onStopCalled = False
        self.start(False,
                (startAnim,
                 lambda: self.compareImage("testLinearAnimZeroDurationC1"),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 lambda: self.assert_(self.__onStopCalled),
                 lambda: self.assert_(not(self.__anim.isRunning()))
                ))
        self.__anim = None

    def testPingPongStopAnim(self):
        def forth():
            anim = avg.LinearAnim(self.__node, 'pos', 100, (50, 100),
                (100, 100), False, None, back)
            anim.start()
            
        def back():
            anim = avg.LinearAnim(self.__node, 'pos', 100, (100, 100),
                (50, 100), False, None, forth)
            anim.start()

        self.initScene()
        self.start(False,
                (forth,
                 lambda: self.delay(300),
                ))

    def testPointAnim(self):
        self._testPointAnim(
                startPos=(0, 0),
                endPos=(100, 40),
                keepAttrPos=(50, 20),
                startPosImgSrc="testPointAnim1",
                endPosImgSrc="testPointAnim2",
                keepAttrPosImgSrc="testPointAnim3")

    def testXPosPointAnim(self):
        self._testPointAnim(
                startPos=(0, 0),
                endPos=(80, 0),
                keepAttrPos=(40, 0),
                startPosImgSrc="testXPosPointAnim1",
                endPosImgSrc="testXPosPointAnim2",
                keepAttrPosImgSrc="testXPosPointAnim3")

    def testYPosPointAnim(self):
        self._testPointAnim(
                startPos=(0, 0),
                endPos=(0, 80),
                keepAttrPos=(0, 40),
                startPosImgSrc="testYPosPointAnim1",
                endPosImgSrc="testYPosPointAnim2",
                keepAttrPosImgSrc="testYPosPointAnim3")

    def testIntAnim(self):
        self.initScene()
        self.__doubleAnim = avg.LinearAnim(self.__node, "x", 300, 0, 100, True)
        self.__pointAnim = avg.LinearAnim(self.__node, "pos", 200, avg.Point2D(0,0), 
                avg.Point2D(100,40), True)
        self.start(False,
                (self.__doubleAnim.start,
                 lambda: self.delay(100),
                 lambda: self.compareImage("testIntAnim1"),
                 self.__doubleAnim.abort,
                 self.__pointAnim.start,
                 lambda: self.delay(100),
                 lambda: self.compareImage("testIntAnim2"),
                ))

    def testEaseInOutAnim(self):
        self.initScene()
        curAnim = avg.EaseInOutAnim(self.__node, "x", 300, 0, 100, 100, 100, False)
        self.testAnimType(curAnim, "testEaseInOutAnimC")

    def testContinuousAnim(self):
        def startAnim():
            self.__animStarted = True

        def stopAnim():
            self.__animStopped = True

        def reset():
            self.__animStarted = False
            self.__animStopped = False

        self.initScene()
        self.__anim = avg.ContinuousAnim(self.__node, "angle", 0, 2*math.pi, 
            False, startAnim, stopAnim)
        self.__linearAnim = avg.LinearAnim(self.__node, "angle", 1000, math.pi, math.pi)

        self.__animStarted = False
        self.__animStopped = False
        self.start(False,
                (self.__anim.start,
                 lambda: self.assert_(self.__animStarted),
                 lambda: self.compareImage("testContinuousAnim1"),
                 self.__anim.abort,
                 lambda: self.assert_(self.__animStopped),
                 reset,
                 self.__anim.start,
                 self.__linearAnim.start,
                 lambda: self.assert_(self.__animStopped),
                 lambda: self.compareImage("testContinuousAnim2"),
                 self.__linearAnim.abort,
                ))

    def testWaitAnim(self):
        def animStopped():
            self.__endCalled = True

        def startAnim():
            self.anim = avg.WaitAnim(200, animStopped, None)
            self.anim.start()

        self.initScene()
        self.__endCalled = False
        self.start(False,
                (startAnim, 
                 lambda: self.assert_(self.anim.isRunning()),
                 lambda: self.delay(200),
                 lambda: self.assert_(not(self.anim.isRunning())),
                 lambda: self.assert_(self.__endCalled)
                ))

    def testParallelAnim(self):
        def animStopped():
            self.__endCalled = True

        def startFireForgetAnim():
            avg.ParallelAnim(
                    [ avg.LinearAnim(self.nodes[0], "x", 200, 0, 60),
                      avg.LinearAnim(self.nodes[1], "x", 200, 0, 120)
                    ]).start()

        def startAnim():
            self.anim = avg.ParallelAnim(
                    [ avg.LinearAnim(self.nodes[0], "x", 200, 0, 60),
                      avg.LinearAnim(self.nodes[1], "x", 400, 0, 120),
                      avg.EaseInOutAnim(self.nodes[2], "x", 400, 0, 120, 100, 100)
                    ], None, animStopped)
            self.__endCalled = False
            self.anim.start()

        def startTimedAnim():
            self.anim = avg.ParallelAnim(
                    [ avg.LinearAnim(self.nodes[0], "x", 200, 0, 60),
                      avg.LinearAnim(self.nodes[1], "x", 400, 0, 120),
                    ], None, animStopped, 200)
            self.__endCalled = False
            self.anim.start()
            
        def abortAnim():
            self.anim.abort()

        def deleteAnim():
            self.anim = None

        root = self.loadEmptyScene()
        self.nodes = []
        for i in range(3):
            node = avg.ImageNode(id=str(i), pos=(64, i*20), href="rgb24-64x64.png")
            root.appendChild(node)
            self.nodes.append(node)
        player.setFakeFPS(10)
        self.__endCalled = False
        self.start(False,
                (startFireForgetAnim,
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 2),
                 None,
                 startAnim,
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 3),
                 lambda: self.compareImage("testParallelAnimC1"),
                 lambda: self.assert_(self.anim.isRunning()),
                 lambda: self.delay(200),
                 lambda: self.assert_(not(self.anim.isRunning())),
                 lambda: self.compareImage("testParallelAnimC2"),
                 lambda: self.assert_(self.__endCalled),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 startAnim,
                 abortAnim,
                 lambda: self.compareImage("testParallelAnimC3"),
                 lambda: self.assert_(self.__endCalled),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 startTimedAnim,
                 lambda: self.delay(200),
                 lambda: self.assert_(self.__endCalled),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 startAnim
                ))
        self.nodes = []

    def testParallelAnimRegistry(self):
        def makeAnims():
            avg.ParallelAnim(
                        [ avg.LinearAnim(self.__node, "x", 200, 0, 60),
                          avg.LinearAnim(self.__node, "y", 200, 0, 120)
                        ]).start()
            
            avg.LinearAnim(self.__node, "x", 300, 0, 100, False, None).start()
        
        self.initScene()
        self.start(False,
                (makeAnims,
                 None
                ))

    def testStateAnim(self):
        def state1StopCallback():
            self.__state1StopCallbackCalled = True

        def state2StartCallback():
            if self.__state1StopCallbackCalled:
                self.__stop1Start2CallbackOrder = True
            self.__state2StartCallbackCalled = True

        def makeAnim():
            self.anim = avg.StateAnim(
                    [avg.AnimState("STATE1", avg.LinearAnim(self.__node, "x", 200, 
                            0, 100, False, None, state1StopCallback), "STATE2"),
                     avg.AnimState("STATE2", avg.LinearAnim(self.__node, "x", 200, 
                            100, 50, False, state2StartCallback), "STATE3"),
                     avg.AnimState("STATE3", avg.WaitAnim())
                    ])
#            self.anim.setDebug(True)

        def killAnim():
            self.anim = None

        self.initScene()
        self.__state1StopCallbackCalled = False
        self.__state2StartCallbackCalled = False
        self.__stop1Start2CallbackOrder = False
        self.start(False,
                (makeAnim,
                 lambda: self.compareImage("testStateAnimC1"),
                 lambda: self.anim.setState("STATE1"),
                 None,
                 lambda: self.compareImage("testStateAnimC2"),
                 lambda: self.assertEqual(self.anim.getState(), "STATE2"),
                 lambda: self.compareImage("testStateAnimC3"),
                 lambda: self.assert_(self.__state1StopCallbackCalled),
                 lambda: self.assert_(self.__state2StartCallbackCalled),
                 lambda: self.assert_(self.__stop1Start2CallbackOrder),
                 lambda: self.assertEqual(self.anim.getState(), "STATE3"),
                 lambda: self.compareImage("testStateAnimC4"),
                 lambda: self.anim.setState("STATE1"),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 1),
                 lambda: self.compareImage("testStateAnimC5"),
                 killAnim,
#                 lambda: player.getTestHelper().dumpObjects()
                ))

    def testNonNodeAttrAnim(self):
        class GenericClass(object):
            def __init__(self):
                self.numberValue = 0
                self.pointValue = avg.Point2D(0.0, 0.0)

        def on1Stop():
            self.__onStopCalled = True

        def on2Start():
            self.__onStopBeforeOnStart = self.__onStopCalled

        self.loadEmptyScene()
        player.setFakeFPS(10)
        genericObject1 = GenericClass()
        genericObject2 = genericObject1
        genericObject3 = GenericClass()
        anim1 = avg.LinearAnim(genericObject1, "numberValue", 1000, 0, 100,
                False, None, on1Stop)
        anim2 = avg.LinearAnim(genericObject2, "numberValue", 1200, 0, 200,
                False, on2Start)
        anim3 = avg.LinearAnim(genericObject1, "pointValue", 800, (0, 0), (100, 200))
        anim4 = avg.LinearAnim(genericObject3, "numberValue", 400, 0, 42)
        self.__onStopCalled = False
        self.__onStopBeforeOnStart = False
        self.start(False,
                (lambda: anim1.start(),
                 lambda: self.assert_(anim1.isRunning()),
                 lambda: self.assert_(not(self.__onStopCalled)),
                 lambda: anim2.start(),
                 lambda: self.assert_(self.__onStopBeforeOnStart),
                 lambda: self.assert_(not(anim1.isRunning())),
                 lambda: self.assert_(anim2.isRunning()),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 1),
                 lambda: anim3.start(),
                 lambda: self.assert_(anim2.isRunning()),
                 lambda: self.assert_(anim3.isRunning()),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 2),
                 lambda: anim4.start(),
                 lambda: self.assert_(anim4.isRunning()),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 3),
                 lambda: self.delay(200),
                 lambda: self.assertEqual(avg.getNumRunningAnims(), 0),
                 lambda: self.assert_(genericObject1.numberValue == 200),
                 lambda: self.assert_(genericObject2.pointValue == (100, 200)),
                 lambda: self.assert_(genericObject3.numberValue == 42)
                ))
        anim1 = None
        anim2 = None
        anim3 = None
        anim4 = None
        genericObject1 = None
        genericObject2 = None
        genericObject3 = None


    def _testPointAnim(self, startPos, endPos, keepAttrPos, startPosImgSrc, endPosImgSrc,
            keepAttrPosImgSrc):
        def startAnim():
            self.__anim.start()

        def startKeepAttr():
            self.__node.pos = keepAttrPos
            self.__anim.start(True)

        self.initScene()
        self.__anim = avg.LinearAnim(self.__node, "pos", 200, avg.Point2D(startPos),
                avg.Point2D(endPos), False)
        self.start(False,
                (startAnim,
                 lambda: self.compareImage(startPosImgSrc),
                 lambda: self.compareImage(endPosImgSrc),
                 None,
                 lambda: self.assert_(not(self.__anim.isRunning())),
                 startKeepAttr,
                 lambda: self.compareImage(keepAttrPosImgSrc),
                ))


def animTestSuite(tests):
    availableTests = (
        "testLinearAnim",
        "testAnimRegistry",
        "testFadeIn",
        "testFadeOut",
        "testNonExistentAttributeAnim",
        "testLinearAnimZeroDuration",
        "testPingPongStopAnim",
        "testPointAnim",
        "testXPosPointAnim",
        "testYPosPointAnim",
        "testEaseInOutAnim",
        "testIntAnim",
        "testContinuousAnim",
        "testWaitAnim",
        "testParallelAnim",
        "testParallelAnimRegistry",
        "testStateAnim",
        "testNonNodeAttrAnim"
        )
    return createAVGTestSuite(availableTests, AnimTestCase, tests)

