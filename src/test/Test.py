#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, syslog, time, os
import tempfile
sys.path += ['../python/.libs', '../python']
import avg
import anim

CREATE_BASELINE_IMAGES = False
BASELINE_DIR = "baseline"
RESULT_DIR = "resultimages"

ourSaveDifferences = True

class LoggerTestCase(unittest.TestCase):
    def test(self):
        self.Log = avg.Logger.get()
        self.Log.setCategories(self.Log.APP |
                  self.Log.WARNING
#                  self.Log.PROFILE |
#                  self.Log.PROFILE_LATEFRAMES |
#                  self.Log.CONFIG |
#                  self.Log.MEMORY | 
#                  self.Log.BLTS    |
#                  self.Log.EVENTS
#                  self.Log.EVENTS2
                  )
        myTempFile = os.path.join(tempfile.gettempdir(), "testavg.log")
        try:
            os.remove(myTempFile)
        except OSError:
            pass
        self.Log.setFileDest(myTempFile)
        self.Log.trace(self.Log.APP, "Test file log entry.")
        readLog = file(myTempFile, "r").readlines()
        self.assert_(len(readLog) == 1)
        myBaseLine = "APP: Test file log entry."
        self.assert_(readLog[0].find(myBaseLine) >= 0)
        stats = os.stat(myTempFile)
        # Windows text files have two chars for linefeed
        self.assert_(stats.st_size in [50, 51])
        
        self.Log.setSyslogDest(syslog.LOG_USER, syslog.LOG_CONS)
        self.Log.trace(self.Log.APP, "Test syslog entry.")
        self.Log.setConsoleDest()

class NodeTestCase(unittest.TestCase):
    def testAttributes(self):
        self.Image = avg.Image()
        assert self.Image.id == ""
        self.Image.x = 10
        self.Image.x += 1
        assert self.Image.x == 11

class AVGTestCase(unittest.TestCase):
    def __init__(self, testFuncName, engine, bpp):
        self.__engine = engine
        self.__bpp = bpp
        self.__testFuncName = testFuncName
        self.Log = avg.Logger.get()
        unittest.TestCase.__init__(self, testFuncName)
    def setUpVideo(self):
        Player.setDisplayEngine(self.__engine)
        Player.setResolution(0, 0, 0, self.__bpp)
        Player.setOGLOptions(UsePOW2Textures, YCbCrMode, UseRGBOrder, UsePixelBuffers, 1)
    def setUp(self):
        self.setUpVideo()
        print "-------- ", self.__testFuncName, " --------"
    def start(self, filename, actions):
        self.assert_(Player.isPlaying() == 0)
        Player.loadFile(filename)
        self.actions = actions
        self.curFrame = 0
        Player.setInterval(0, self.nextAction)
        Player.setFramerate(100)
        Player.play()
        self.assert_(Player.isPlaying() == 0)
    def nextAction(self):
        self.actions[self.curFrame]()
#        print (self.curFrame)
        self.curFrame += 1
    def compareImage(self, fileName, warn):
        global CREATE_BASELINE_IMAGES
        Bmp = Player.screenshot()
        if CREATE_BASELINE_IMAGES:
            Bmp.save(BASELINE_DIR+"/"+fileName+".png")
        else:
            try:
                BaselineBmp = avg.Bitmap(BASELINE_DIR+"/"+fileName+".png")
                NumPixels = Player.getTestHelper().getNumDifferentPixels(Bmp, BaselineBmp)
                if (NumPixels > 20):
                    if ourSaveDifferences:
                        Bmp.save(RESULT_DIR+"/"+fileName+".png")
                        BaselineBmp.save(RESULT_DIR+"/"+fileName+"_baseline.png")
                        Bmp.subtract(BaselineBmp)
                        Bmp.save(RESULT_DIR+"/"+fileName+"_diff.png")
                    self.Log.trace(self.Log.WARNING, "Image compare: "+str(NumPixels)+
                            " bright pixels.")
                    if warn:
                        self.Log.trace(self.Log.WARNING, "Image "+fileName
                                +" differs from original.")
                    else:
                        self.assert_(False)
            except RuntimeError:
                Bmp.save(RESULT_DIR+"/"+fileName+".png")
                self.Log.trace(self.Log.WARNING, "Could not load image "+fileName+".png")
                self.assert_(False)

def keyUp():
    print "keyUp"

def keyDown():
    print "keyDown"
    Event = Player.getCurEvent()
    print Event
    print "  Type: "+str(Event.type)
    print "  keystring: "+Event.keystring
    print "  scancode: "+str(Event.scancode)
    print "  keycode: "+str(Event.keycode)
    print "  modifiers: "+str(Event.modifiers)

def dumpMouseEvent():
    Event = Player.getCurEvent()
    print Event
    print "  type: "+str(Event.type)
    print "  leftbuttonstate: "+str(Event.leftbuttonstate)
    print "  middlebuttonstate: "+str(Event.middlebuttonstate)
    print "  rightbuttonstate: "+str(Event.rightbuttonstate)
    print "  position: "+str(Event.x)+","+str(Event.y)
    print "  node: "+Event.node.id

mainMouseDownCalled = False
mouseMove1Called = False
mouseUp1Called = False
mouseDown1Called = False
mouseOver1Called = False
mouseOut1Called = False
divMouseDownCalled = False
mouseDown2Called = False
obscuredMouseDownCalled = False
deactMouseDownCalled = False
deactMouseOverLate = False
deactMouseOverCalled = False

def mainMouseUp():
    global mainMouseUpCalled
    mainMouseUpCalled = True

def mainMouseDown():
    global mainMouseDownCalled
    mainMouseDownCalled = True

def onMouseMove1():
    print "onMouseMove1"

def onMouseUp1():
    global mouseUp1Called
    mouseUp1Called = True

def onMouseDown1():
    global mouseDown1Called
    mouseDown1Called = True

def onMouseOver1():
    global mouseOver1Called
    mouseOver1Called = True

def onMouseOut1():
    global mouseOut1Called
    mouseOut1Called = True

def onDivMouseDown():
    global divMouseDownCalled
    divMouseDownCalled = True

def onMouseDown2():
    global mouseDown2Called
    mouseDown2Called = True

def onObscuredMouseDown():
    global obscuredMouseDownCalled
    obscuredMouseDownCalled = True

def onErrMouseOver():
    undefinedFunction()

def onDeactMouseDown():
    global deactMouseDownCalled
    deactMouseDownCalled = True

def onDeactMouseOver():
    global deactMouseDownCalled
    global deactMouseOverLate
    global deactMouseOverCalled
    deactMouseOverLate = deactMouseDownCalled
    deactMouseOverCalled = True

def onDeactMouseMove():
    print("move")

def onDeactMouseOut():
    print("out")

mainCaptureMouseDownCalled = False
captureMouseDownCalled = False

def onMainCaptureMouseDown():
    global mainCaptureMouseDownCalled
    mainCaptureMouseDownCalled = True

def onCaptureMouseDown():
    global captureMouseDownCalled
    captureMouseDownCalled = True

class PlayerTestCase(AVGTestCase):
    def __init__(self, testFuncName, engine, bpp):
        AVGTestCase.__init__(self, testFuncName, engine, bpp)
    def testImage(self):    
        def loadNewFile():
            Player.getElementByID("test").href = "rgb24alpha-64x64.png"
            Player.getElementByID("testhue").href = "rgb24alpha-64x64.png"
        def getBitmap():
            node = Player.getElementByID("test")
            Bmp = node.getBitmap()
            self.assert_(Bmp.getSize() == (65,65))
            self.assert_(Bmp.getFormat() == avg.R8G8B8 or Bmp.getFormat() == avg.B8G8R8)
        self.start("image.avg",
                (lambda: self.compareImage("testimg", False), 
                 getBitmap,
                 loadNewFile, 
                 lambda: self.compareImage("testimgload", False),
                 lambda: Player.setGamma(0.7, 0.7, 0.7),
                 lambda: Player.setGamma(1.0, 1.0, 1.0),
                 Player.stop))
    def testError(self):
        Player.loadFile("image.avg")
        Player.setTimeout(1, lambda: undefinedFunction)
        Player.setTimeout(50, Player.stop)
        try:
            Player.play()
        except NameError:
            self.assert_(1)
        else:
            self.assert_(0)
    def testExceptionInTimeout(self):
        def throwException():
            raise ZeroDivisionError
        try:
            self.start("image.avg",
                    (throwException,
                     Player.stop))
        except ZeroDivisionError:
            self.assert_(1)
        else:
            self.assert_(0)
    def testInvalidImageFilename(self):
        def activateNode():
            Player.getElementByID("enclosingdiv").active = 1
        self.start("invalidfilename.avg",
                (activateNode,
                 Player.stop))
    def testEvents(self):
        def getMouseState():
            Event = Player.getMouseState()
        def testInactiveDiv():
            Player.getElementByID("div1").active = False
            Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                70, 70, 1)
        Helper = Player.getTestHelper()
        global mainMouseDownCalled
        global mouseMove1Called
        global mouseUp1Called
        global mouseDown1Called
        global mouseOver1Called
        global mouseOut1Called
        global divMouseDownCalled
        global mouseDown2Called
        global obscuredMouseDownCalled
        global deactMouseOverCalled
        global deactMouseOverLate
        global deactMouseDownCalled

        self.start("events.avg", 
                (lambda: self.compareImage("testEvents", False),
                 lambda: Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(mouseDown1Called and mouseOver1Called 
                        and mainMouseDownCalled),
                 getMouseState,
                 lambda: Helper.fakeMouseEvent(avg.MOUSEBUTTONUP, True, False, False,
                        12, 12, 1),
                 lambda: self.assert_(mouseUp1Called and mainMouseUpCalled),
                 lambda: Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                        70, 70, 1),
                 lambda: self.assert_(mouseDown2Called and divMouseDownCalled and 
                        mouseOut1Called and not(obscuredMouseDownCalled)),
                 testInactiveDiv,
                 lambda: self.assert_(obscuredMouseDownCalled),
                 # Test if deactivation between mouse click and mouse out works.
                 lambda: Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                        70, 10, 1),
                 lambda: self.assert_(deactMouseOverCalled and not(deactMouseOverLate)),
                 # XXX
                 # - errMouseOver
                 Player.stop))
    def testEventCapture(self):
        def captureEvent():
            global captureMouseDownCalled
            Helper = Player.getTestHelper()
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").setEventCapture()
            Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                    100, 10, 1)
        def noCaptureEvent():
            global captureMouseDownCalled
            Helper = Player.getTestHelper()
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").releaseEventCapture()
            Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                    100, 10, 1)
        global captureMouseDownCalled
        global mainCaptureMouseDownCalled
        Helper = Player.getTestHelper()
        self.start("eventcapture.avg",
                (lambda: Helper.fakeMouseEvent(avg.MOUSEBUTTONDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(captureMouseDownCalled),
                 captureEvent,
                 lambda: self.assert_(captureMouseDownCalled and 
                        mainCaptureMouseDownCalled),
                 noCaptureEvent,
                 lambda: self.assert_(not(captureMouseDownCalled) and 
                        mainCaptureMouseDownCalled),
                 Player.stop))
    def testTimeouts(self):
        self.timeout1called = False
        self.timeout2called = False
        def timeout1():
            Player.clearInterval(self.timeout1ID)
            Player.clearInterval(self.timeout2ID)
            self.timeout1called = True
        def timeout2():
            self.timeout2called = True
        def wait():
            pass
        def setupTimeouts():
            self.timeout1ID = Player.setTimeout(0, timeout1)
            self.timeout2ID = Player.setTimeout(1, timeout2)
        self.start("image.avg",
                (setupTimeouts,
                 wait,
                 lambda: self.assert_(self.timeout1called),
                 lambda: self.assert_(not(self.timeout2called)),
                 Player.stop))
    def testEventErr(self):
        Player.loadFile("errevent.avg")
        Player.setTimeout(10, Player.stop)
        try:
            Player.play()
        except NameError:
            print("(Intentional) NameError caught")
            self.assert_(1)
    def testHugeImage(self):
        def moveImage():
            Player.getElementByID("mainimg").x -= 2500
        self.start("hugeimage.avg",
                (lambda: self.compareImage("testHugeImage0", False),
                 moveImage,
                 lambda: self.compareImage("testHugeImage1", False),
                 Player.stop))
    def testPanoImage(self):
        def changeProperties():
            node = Player.getElementByID("pano")
            node.sensorheight=10
            node.sensorwidth=15
            node.focallength=25
        def loadImage():
            node = Player.getElementByID("pano")
            node.href = "rgb24-65x65.png"
        self.start("panoimage.avg",
                (lambda: self.compareImage("testPanoImage", False),
                 lambda: time.sleep,
                 changeProperties,
                 loadImage,
                 Player.stop))
    def testBroken(self):
        def testBrokenFile(filename):
            exceptionRaised = False
            try:
                Player.loadFile(filename)
            except RuntimeError:
                exceptionRaised = True
            self.assert_(exceptionRaised)
        testBrokenFile("noxml.avg")
        testBrokenFile("noavg.avg")
        testBrokenFile("noavg2.avg")
    def testMove(self):
        def moveit():
            node = Player.getElementByID("nestedimg1")
            node.x += 50
            node.opacity -= 0.7
            node = Player.getElementByID("nestedavg")
            node.x += 50
        self.start("avg.avg",
                (lambda: self.compareImage("testMove1", False),
                 moveit,
                 lambda: self.compareImage("testMove2", False),
                 Player.stop))
    def testBlend(self):
        self.start("blend.avg",
                (lambda: self.compareImage("testBlend", False),
                 Player.stop))
    def testCropImage(self):
        def moveTLCrop():
            node = Player.getElementByID("img")
            node.x = -20
            node.y = -20
        def moveBRCrop():
            node = Player.getElementByID("img")
            node.x = 60
            node.y = 40
        def moveTLNegative():
            node = Player.getElementByID("img")
            node.x = -60
            node.y = -50
        def moveBRGone():
            node = Player.getElementByID("img")
            node.x = 140
            node.y = 100
        self.start("crop2.avg",
                (lambda: self.compareImage("testCropImage1", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropImage2", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropImage3", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropImage4", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropImage5", False),
                 Player.stop))
    def testCropMovie(self):
        def playMovie():
            node = Player.getElementByID("movie")
            node.play()
        def moveTLCrop():
            node = Player.getElementByID("movie")
            node.x = -20
            node.y = -20
        def moveBRCrop():
            node = Player.getElementByID("movie")
            node.x = 60
            node.y = 40
        def moveTLNegative():
            node = Player.getElementByID("movie")
            node.x = -60
            node.y = -50
        def moveBRGone():
            node = Player.getElementByID("movie")
            node.x = 140
            node.y = 100
        self.start("crop.avg",
                (playMovie,
                 lambda: self.compareImage("testCropMovie1", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropMovie2", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropMovie3", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropMovie4", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropMovie5", False),
                 Player.stop))
    def testWarp(self):
        def moveVertex():
            node = Player.getElementByID("testtiles")
            pos = node.getWarpedVertexCoord(1,1)
            pos.x += 0.06
            pos.y += 0.06
            node.setWarpedVertexCoord(1,1,pos)
            node = Player.getElementByID("clogo1")
            pos = node.getWarpedVertexCoord(0,0)
            pos.x += 0.06
            pos.y += 0.06
            node.setWarpedVertexCoord(0,0,pos)
        def flip():
            node = Player.getElementByID("testtiles")
            for y in range(node.getNumVerticesY()):
                for x in range(node.getNumVerticesX()):
                    pos = node.getOrigVertexCoord(x,y)
                    pos.x = 1-pos.x
                    node.setWarpedVertexCoord(x,y,pos)
        self.start("video.avg",
                (lambda: Player.getElementByID("clogo1").play(),
                 lambda: self.compareImage("testWarp1", False),
                 moveVertex,
                 lambda: self.compareImage("testWarp2", True),
                 flip,
                 lambda: self.compareImage("testWarp3", False),
                 Player.stop))
    def testWords(self):
        def changeText():
            node = Player.getElementByID("cbasetext")
            str = "blue"
            node.text = str
            node.color = "404080"
            node.x += 10
        def changeHeight():
            node = Player.getElementByID("cbasetext")
            node.height = 28
        def activateText():
            Player.getElementByID('cbasetext').active = 1
        def deactivateText():
            Player.getElementByID('cbasetext').active = 0
        def changeFont():
            node = Player.getElementByID("cbasetext")
            node.font = "Times New Roman"
            node.height = 0
            node.size = 30
        def changeFont2():
            node = Player.getElementByID("cbasetext")
            node.size = 18
        def changeUnicodeText():
            Player.getElementByID("dynamictext").text = "Arabic nonsense: ﯿﭗ"
        self.start("text.avg",
                (lambda: self.compareImage("testWords1", True),
                 changeText,
                 changeHeight,
                 deactivateText,
                 lambda: self.compareImage("testWords2", True),
                 activateText,
                 changeFont,
                 lambda: self.compareImage("testWords3", True),
                 changeFont2,
                 changeUnicodeText,
                 lambda: self.compareImage("testWords4", True),
                 Player.stop))
    def testVideo(self):
        def seek():
            Player.getElementByID("clogo2").seekToFrame(100)
        def foo():
            pass
        def newHRef():
            node = Player.getElementByID("clogo2")
            node.href = "h264-48x48.h264"
            node.play()
        def move():
            node = Player.getElementByID("clogo2")
            node.x += 30
        def activateclogo():
            Player.getElementByID('clogo').active=1
        def deactivateclogo():
            Player.getElementByID('clogo').active=0
        self.start("video.avg",
                (lambda: self.compareImage("testVideo1", False),
                 lambda: Player.getElementByID("clogo2").play(),
                 seek,
                 lambda: self.compareImage("testVideo2", False),
                 lambda: Player.getElementByID("clogo2").pause(),
                 lambda: self.compareImage("testVideo3", False),
                 lambda: Player.getElementByID("clogo2").play(),
                 lambda: self.compareImage("testVideo4", False),
                 newHRef,
                 lambda: Player.getElementByID("clogo1").play(),
                 lambda: self.compareImage("testVideo5", False),
                 move,
                 lambda: Player.getElementByID("clogo").pause(),
                 lambda: self.compareImage("testVideo6", False),
                 deactivateclogo,
                 lambda: self.compareImage("testVideo7", False),
                 activateclogo,
                 lambda: self.compareImage("testVideo8", False),
                 lambda: Player.getElementByID("clogo").stop(),
                 lambda: self.compareImage("testVideo9", False),
                 Player.stop))
    def testAnim(self):
        def onStart():
            Player.setTimeout(10, startAnim)
            Player.setTimeout(380, startSplineAnim)
            Player.setTimeout(800, lambda: self.compareImage("testAnim3", False))
            Player.setTimeout(850, Player.stop)
        def startAnim():
            def onStop():
                self.__animStopped = True
            self.compareImage("testAnim1", False)
            anim.fadeOut(Player.getElementByID("nestedimg2"), 200)
            Player.getElementByID("nestedimg1").opacity = 0
            anim.fadeIn(Player.getElementByID("nestedimg1"), 200, 1)
            anim.LinearAnim(Player.getElementByID("nestedimg1"), "x", 
                    200, 0, 100, 0, onStop)
        def startSplineAnim():
            self.assert_(self.__animStopped)
            self.compareImage("testAnim2", False)
            anim.SplineAnim(Player.getElementByID("mainimg"), "x", 
                    200, 100, -400, 10, 0, 0, None)
            anim.SplineAnim(Player.getElementByID("mainimg"), "y", 
                    200, 100, 0, 10, -400, 1, None)
        self.__animStopped = False
        anim.init(Player)
        Player.loadFile("avg.avg")
        Player.setTimeout(1, onStart)
        Player.setVBlankFramerate(1)
        Player.play()
        
    def testImgDynamics(self):
        def createImg():
            node = Player.createNode("<image href='rgb24-64x64.png'/>")
            node.id = "newImage"
            node.x = 10
            node.y = 20
            node.opacity = 0.666
            node.angle = 0.1
            node.blendmode = "add"
#            print node.toXML()
            self.rootNode = Player.getRootNode()
            self.rootNode.addChild(node)
            exceptionRaised=False
            try:
                node.id = "bork"
            except RuntimeError:
                exceptionRaised=True
            self.assert_(exceptionRaised)
            self.assert_(self.rootNode.indexOf(Player.getElementByID("newImage")) == 0)
        def removeImg():
            self.imgNode = Player.getElementByID("newImage")
            self.rootNode.removeChild(
                    self.rootNode.indexOf(self.imgNode))
            self.assert_(Player.getElementByID("newImage") == None)
        def reAddImg():
            self.rootNode.addChild(self.imgNode)
        Player.loadFile("empty.avg")
        createImg()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createImg,
                 lambda: self.compareImage("testImgDynamics1", False),
                 removeImg,
                 lambda: self.compareImage("testImgDynamics2", False),
                 reAddImg,
                 lambda: self.compareImage("testImgDynamics3", False),
                 Player.stop))

    def testVideoDynamics(self):
        def createVideo():
            node = Player.createNode("<video id='newVideo' href='mpeg1-48x48.mpg'/>")
            self.rootNode = Player.getRootNode()
            self.rootNode.addChild(node)
            node.play()
        def removeVideo():
            self.videoNode = Player.getElementByID("newVideo")
            self.rootNode.removeChild(self.rootNode.indexOf(self.videoNode))
        def reAddVideo():
            self.rootNode.addChild(self.videoNode)
            exceptionRaised = False
            try:
                self.rootNode.addChild(self.videoNode)
            except RuntimeError:
                exceptionRaised = True
            self.assert_(exceptionRaised)
            self.videoNode.play()
        def foo():
            pass
        Player.loadFile("empty.avg")
        createVideo()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createVideo,
                 lambda: self.compareImage("testVideoDynamics1", False),
                 removeVideo,
                 lambda: self.compareImage("testVideoDynamics2", False),
                 reAddVideo,
                 lambda: self.compareImage("testVideoDynamics3", False),
                 Player.stop))
    def testWordsDynamics(self):
        def createWords():
            node = Player.createNode("<words id='newWords' text='test'/>")
            node.font="times new roman"
            node.size=12
            node.parawidth=200
            node.x=10
            self.rootNode = Player.getRootNode()
            self.rootNode.addChild(node)
        def removeWords():
            self.wordsNode = Player.getElementByID("newWords")
            self.rootNode.removeChild(self.rootNode.indexOf(self.wordsNode))
        def reAddWords():
            self.rootNode.addChild(self.wordsNode)
            self.wordsNode.text='test2'
        Player.loadFile("empty.avg")
        createWords()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createWords,
                 lambda: self.compareImage("testWordsDynamics1", True),
                 removeWords,
                 lambda: self.compareImage("testWordsDynamics2", True),
                 reAddWords,
                 lambda: self.compareImage("testWordsDynamics3", True),
                 Player.stop))
    def testCameraDynamics(self):
        def createCamera():
            node = Player.createNode("<camera id='newCamera' device='/dev/video1394/0' mode='640x480_RGB' framerate='15'/>")
            self.rootNode = Player.getRootNode()
            self.rootNode.addChild(node)
        def removeCamera():
            self.cameraNode = Player.getElementByID("newCamera")
            self.rootNode.removeChild(self.rootNode.indexOf(self.cameraNode))
        def reAddCamera():
            self.rootNode.addChild(self.cameraNode)
        Player.loadFile("empty.avg")
        createCamera()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createCamera,
                 removeCamera,
                 reAddCamera,
                 Player.stop))
    def testPanoDynamics(self):
        def createPano():
            node = Player.createNode("<panoimage id='newPano' href='panoimage.png' sensorwidth='4.60' sensorheight='3.97' focallength='12' width='160' height='120'/>")
            self.rootNode = Player.getRootNode()
            self.rootNode.addChild(node)
        def removePano():
            self.panoNode = Player.getElementByID("newPano")
            self.rootNode.removeChild(self.rootNode.indexOf(self.panoNode))
        def reAddPano():
            self.rootNode.addChild(self.panoNode)
        Player.loadFile("empty.avg")
        createPano()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createPano,
                 lambda: self.compareImage("testPanoDynamics1", False),
                 removePano,
                 lambda: self.compareImage("testPanoDynamics2", False),
                 reAddPano,
                 lambda: self.compareImage("testPanoDynamics3", False),
                 Player.stop))
    def testDivDynamics(self):
        def createDiv():
            node = Player.createNode("<div id='newDiv'><image id='nestedImg' href='rgb24-64x64.png'/></div>")
            self.rootNode = Player.getRootNode()
            self.rootNode.addChild(node)
        def removeDiv():
            self.divNode = Player.getElementByID("newDiv")
            self.rootNode.removeChild(self.rootNode.indexOf(self.divNode))
        def reAddDiv():
            node = Player.createNode("<image id='img2' href='rgb24-64x64.png' x='64'/>")
            self.divNode.addChild(node)
            self.rootNode.addChild(self.divNode)
        Player.loadFile("empty.avg")
        createDiv()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createDiv,
                 lambda: self.compareImage("testDivDynamics1", False),
                 removeDiv,
                 lambda: self.compareImage("testDivDynamics2", False),
                 reAddDiv,
                 lambda: self.compareImage("testDivDynamics3", False),
                 Player.stop))
            
def playerTestSuite(engine, bpp):
    def rmBrokenDir():
        try:
            files = os.listdir(RESULT_DIR)
            for file in files:
                os.remove(RESULT_DIR+"/"+file)
        except OSError:
            try:
                os.mkdir(RESULT_DIR)
            except OSError:
                # This can happen on make distcheck (permission denied...)
                global ourSaveDifferences
                ourSaveDifferences = False
    rmBrokenDir()
    suite = unittest.TestSuite()
    suite.addTest(NodeTestCase("testAttributes"))
    suite.addTest(PlayerTestCase("testImage", engine, bpp))
    suite.addTest(PlayerTestCase("testError", engine, bpp))
    suite.addTest(PlayerTestCase("testExceptionInTimeout", engine, bpp))
    suite.addTest(PlayerTestCase("testInvalidImageFilename", engine, bpp))
    suite.addTest(PlayerTestCase("testEvents", engine, bpp))
    suite.addTest(PlayerTestCase("testEventCapture", engine, bpp))
    suite.addTest(PlayerTestCase("testTimeouts", engine, bpp))
    suite.addTest(PlayerTestCase("testEventErr", engine, bpp))
    suite.addTest(PlayerTestCase("testHugeImage", engine, bpp))
    suite.addTest(PlayerTestCase("testPanoImage", engine, bpp))
    suite.addTest(PlayerTestCase("testBroken", engine, bpp))
    suite.addTest(PlayerTestCase("testMove", engine, bpp))
    suite.addTest(PlayerTestCase("testBlend", engine, bpp))
    suite.addTest(PlayerTestCase("testCropImage", engine, bpp))
    suite.addTest(PlayerTestCase("testCropMovie", engine, bpp))
    suite.addTest(PlayerTestCase("testWarp", engine, bpp))
    suite.addTest(PlayerTestCase("testWords", engine, bpp))
    suite.addTest(PlayerTestCase("testVideo", engine, bpp))
    suite.addTest(PlayerTestCase("testAnim", engine, bpp))
    suite.addTest(PlayerTestCase("testImgDynamics", engine, bpp))
    suite.addTest(PlayerTestCase("testVideoDynamics", engine, bpp))
    suite.addTest(PlayerTestCase("testWordsDynamics", engine, bpp))
    suite.addTest(PlayerTestCase("testPanoDynamics", engine, bpp))
    suite.addTest(PlayerTestCase("testCameraDynamics", engine, bpp))
    suite.addTest(PlayerTestCase("testDivDynamics", engine, bpp))
    return suite

def completeTestSuite(engine, bpp):
    suite = unittest.TestSuite()
    suite.addTest(LoggerTestCase("test"))
    suite.addTest(playerTestSuite(engine, bpp))
    return suite

def getBoolParam(paramIndex):
    param = sys.argv[paramIndex].upper()
    if param == "TRUE":
        return True
    elif param == "FALSE":
        return False
    else:
        print "Parameter "+paramIndex+" must be 'True' or 'False'"

if len(sys.argv) == 1:
    engine = avg.OGL
    bpp = 24
    customOGLOptions = False
elif len(sys.argv) == 3 or len(sys.argv) == 7:
    if sys.argv[1] == "OGL":
        engine = avg.OGL
    elif sys.argv[1] == "DFB":
        engine = avg.DFB
    else:
        print "First parameter must be OGL or DFB"
    bpp = int(sys.argv[2])
    if (len(sys.argv) == 7):
        customOGLOptions = True
        UsePOW2Textures = getBoolParam(3)
        s = sys.argv[4]
        if s == "shader":
            YCbCrMode = avg.shader
        elif s == "apple":
            YCbCrMode = avg.apple
        elif s == "mesa":
            YCbCrMode = avg.mesa
        elif s == "none":
            YCbCrMode = avg.none
        else:
            print "Fourth parameter must be shader, apple, mesa or none"
        UseRGBOrder = getBoolParam(5)
        UsePixelBuffers = getBoolParam(6)
    else:
        customOGLOptions = False
else:
    print "Usage: Test.py [<display engine> <bpp>"
    print "               [<UsePOW2Textures> <YCbCrMode> <UseRGBOrder> <UsePixelBuffers>]]"
    sys.exit(1)

if not(customOGLOptions): 
    UsePOW2Textures = False 
    YCbCrMode = avg.shader
    UseRGBOrder = False 
    UsePixelBuffers = True

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)
Player = avg.Player()
runner = unittest.TextTestRunner()
rc = runner.run(completeTestSuite(engine, bpp))
if rc.wasSuccessful():
    sys.exit(0)
else:
    sys.exit(1)

