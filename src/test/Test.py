#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, time, os, platform
import tempfile

try:
    import syslog
    SYSLOG_AVAILABLE = True
except ImportError:
    SYSLOG_AVAILABLE = False

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess. 
sys.path += ['../python/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation.
    
if platform.system() == 'Windows':
    from libavg import avg
else:    
    import avg

SrcDir = os.getenv("srcdir",".")
os.chdir(SrcDir)
if platform.system() == 'Windows':
    from libavg import anim, draggable
else:    
    import anim, draggable

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
#                  self.Log.EVENTS |
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
        
        if SYSLOG_AVAILABLE:
            self.Log.setSyslogDest(syslog.LOG_USER, syslog.LOG_CONS)
            self.Log.trace(self.Log.APP, "Test syslog entry.")
        self.Log.setConsoleDest()

class AVGTestCase(unittest.TestCase):
    def __init__(self, testFuncName, bpp):
        self.__bpp = bpp
        self.__testFuncName = testFuncName
        self.Log = avg.Logger.get()
        unittest.TestCase.__init__(self, testFuncName)
    def setUpVideo(self):
        Player.setResolution(0, 0, 0, self.__bpp)
        if customOGLOptions:
            Player.setOGLOptions(UsePOW2Textures, YCbCrMode, UsePixelBuffers, 1)
    def setUp(self):
        self.setUpVideo()
        print "-------- ", self.__testFuncName, " --------"
    def start(self, filename, actions):
        self.assert_(Player.isPlaying() == 0)
        if filename != None:
            Player.loadFile(filename)
        self.actions = actions
        self.curFrame = 0
        Player.setOnFrameHandler(self.nextAction)
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
                NumPixels = Player.getTestHelper().getNumDifferentPixels(Bmp, 
                        BaselineBmp)
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

def keyUp(Event):
    print "keyUp"

def keyDown(Event):
    print "keyDown"
    print Event
    print "  Type: "+str(Event.type)
    print "  keystring: "+Event.keystring
    print "  scancode: "+str(Event.scancode)
    print "  keycode: "+str(Event.keycode)
    print "  modifiers: "+str(Event.modifiers)

def dumpMouseEvent(Event):
    print Event
    print "  type: "+str(Event.type)
    print "  leftbuttonstate: "+str(Event.leftbuttonstate)
    print "  middlebuttonstate: "+str(Event.middlebuttonstate)
    print "  rightbuttonstate: "+str(Event.rightbuttonstate)
    print "  position: "+str(Event.x)+","+str(Event.y)
    print "  node: "+Event.node.id

mainMouseUpCalled = False
mainMouseDownCalled = False

def mainMouseUp(Event):
    global mainMouseUpCalled
    assert (Event.type == avg.CURSORUP)
    mainMouseUpCalled = True

def mainMouseDown(Event):
    global mainMouseDownCalled
    assert (Event.type == avg.CURSORDOWN)
    mainMouseDownCalled = True

def onErrMouseOver(Event):
    undefinedFunction()

mainCaptureMouseDownCalled = False
captureMouseDownCalled = False

def onMainCaptureMouseDown(Event):
    global mainCaptureMouseDownCalled
    mainCaptureMouseDownCalled = True

def onCaptureMouseDown(Event):
    global captureMouseDownCalled
    captureMouseDownCalled = True
    
class PlayerTestCase(AVGTestCase):
    def __init__(self, testFuncName, bpp):
        AVGTestCase.__init__(self, testFuncName, bpp)
    
    def delay():
        pass
    
    def testImage(self):    
        def loadNewFile():
            Player.getElementByID("test").href = "rgb24alpha-64x64.png"
            Player.getElementByID("testhue").href = "rgb24alpha-64x64.png"
        def getBitmap():
            node = Player.getElementByID("test")
            Bmp = node.getBitmap()
            self.assert_(Bmp.getSize() == (65,65))
            self.assert_(Bmp.getFormat() == avg.R8G8B8X8 or 
                    Bmp.getFormat() == avg.B8G8R8X8)
        def getFramerate():
            framerate = Player.getEffectiveFramerate()
            self.assert_(framerate > 0)
        Player.showCursor(0)
        Player.showCursor(1)
        self.start("image.avg",
                (lambda: self.compareImage("testimg", False), 
                 getBitmap,
                 getFramerate,
                 loadNewFile, 
                 lambda: self.compareImage("testimgload", False),
                 lambda: Player.setGamma(0.7, 0.7, 0.7),
                 lambda: Player.setGamma(1.0, 1.0, 1.0),
                 lambda: Player.showCursor(0),
                 lambda: Player.showCursor(1),
                 Player.stop))

    def testRotate(self):
        def onOuterDown(Event):
            self.onOuterDownCalled = True
        def fakeRotate():
            Player.getElementByID("outer").angle += 0.1
            Player.getElementByID("inner").angle -= 0.1
        def sendEvent(x, y):
            Helper = Player.getTestHelper()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        x, y, 1)
            
        Player.loadFile("rotate.avg")
        Player.getElementByID("outer").setEventHandler(
                avg.CURSORDOWN, avg.MOUSE, onOuterDown) 
        self.onOuterDownCalled = False
        self.start(None,
                (lambda: self.compareImage("testRotate1", False),
                 fakeRotate,
                 lambda: self.compareImage("testRotate1", False),
                 lambda: sendEvent(85, 70),
                 lambda: self.assert_(not(self.onOuterDownCalled)),
                 lambda: sendEvent(85, 75),
                 lambda: self.assert_(self.onOuterDownCalled),
                 Player.stop))
    def testRotate2(self):
        self.start("rotate2.avg",
                (lambda: self.compareImage("testRotate2", False),
                 Player.stop))
    def testRotate3(self):
        self.start("rotate3.avg",
                (lambda: self.compareImage("testRotate3", False),
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

    def testInvalidVideoFilename(self):
        def tryplay():
            exceptionRaised = False
            try:
                Player.getElementByID("brokenvideo").play()
            except e:
                self.assert_(1)
            else:
                self.assert_(0)
        self.start("invalidvideofilename.avg",
                (lambda: tryplay,
                 lambda: Player.getElementByID("brokenvideo").stop(),
                 Player.stop))

    def testEvents(self):
        def getMouseState():
            Event = Player.getMouseState()
        def testInactiveDiv():
            Player.getElementByID("div1").active = False
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                70, 70, 1)
        def disableHandler():
            self.mouseDown1Called = False
            Player.getElementByID("img1").setEventHandler(avg.CURSORDOWN, avg.MOUSE, None)
        def onMouseMove1(Event):
            self.assert_ (Event.type == avg.CURSORMOTION)
            print "onMouseMove1"
        def onMouseUp1(Event):
            self.assert_ (Event.type == avg.CURSORUP)
            self.mouseUp1Called = True
        def onMouseDown1(Event):
            self.assert_ (Event.type == avg.CURSORDOWN)
            self.mouseDown1Called = True
        def onMouseOver1(Event):
            self.assert_ (Event.type == avg.CURSOROVER)
            self.mouseOver1Called = True
        def onMouseOut1(Event):
            self.assert_ (Event.type == avg.CURSOROUT)
            self.mouseOut1Called = True
        def onTouchDown(Event):
            self.touchDownCalled = True
        def onDivMouseDown(Event):
            self.assert_ (Event.type == avg.CURSORDOWN)
            self.divMouseDownCalled = True
        def onMouseDown2(Event):
            self.assert_ (Event.type == avg.CURSORDOWN)
            self.mouseDown2Called = True
        def onObscuredMouseDown(Event):
            self.obscuredMouseDownCalled = True
        def onDeactMouseDown(Event):
            self.deactMouseDownCalled = True
        def onDeactMouseOver(Event):
            self.deactMouseOverLate = self.deactMouseDownCalled
            self.deactMouseOverCalled = True
        def onDeactMouseMove(Event):
            print("move")
        def onDeactMouseOut(Event):
            pass
        def onTiltedMouseDown(Event):
            self.tiltedMouseDownCalled = True
        def neverCalled(Event):
            self.neverCalledCalled = True

        Helper = Player.getTestHelper()
        global mainMouseUpCalled
        global mainMouseDownCalled
        Player.loadFile("events.avg")
        
        self.mouseMove1Called=False
        self.mouseUp1Called=False
        self.mouseDown1Called=False
        self.mouseOver1Called=False
        self.mouseOut1Called=False
        self.touchDownCalled = False
        img1 = Player.getElementByID("img1")
        img1.setEventHandler(avg.CURSORMOTION, avg.MOUSE, onMouseMove1) 
        img1.setEventHandler(avg.CURSORUP, avg.MOUSE, onMouseUp1) 
        img1.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown1) 
        img1.setEventHandler(avg.CURSOROVER, avg.MOUSE, onMouseOver1) 
        img1.setEventHandler(avg.CURSOROUT, avg.MOUSE, onMouseOut1) 
        img1.setEventHandler(avg.CURSORDOWN, avg.TOUCH, onTouchDown) 

        self.neverCalledCalled=False
        hidden = Player.getElementByID("hidden")
        hidden.setEventHandler(avg.CURSORUP, avg.MOUSE, neverCalled)

        self.obscuredMouseDownCalled=False
        obscured = Player.getElementByID("obscured")
        obscured.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onObscuredMouseDown)

        self.divMouseDownCalled=False
        div1 = Player.getElementByID("div1")
        div1.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onDivMouseDown)

        self.mouseDown2Called=False
        img2 = Player.getElementByID("img2")
        img2.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown2)

        self.deactMouseOverCalled=False
        self.deactMouseOverLate=False
        self.deactMouseDownCalled=False
        deact = Player.getElementByID("deact")
        deact.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onDeactMouseDown)
        deact.setEventHandler(avg.CURSOROVER, avg.MOUSE, onDeactMouseOver)
        deact.setEventHandler(avg.CURSOROUT, avg.MOUSE, onDeactMouseOut)
        deact.setEventHandler(avg.CURSORMOTION, avg.MOUSE, onDeactMouseMove)

        self.tiltedMouseDownCalled=False
        Player.getElementByID("tilted").setEventHandler(
                avg.CURSORDOWN, avg.MOUSE, onTiltedMouseDown)

        self.start(None, 
                (lambda: self.compareImage("testEvents", False),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(self.mouseDown1Called and self.mouseOver1Called 
                        and mainMouseDownCalled and not(self.touchDownCalled)),
                 getMouseState,
                 lambda: Helper.fakeMouseEvent(avg.CURSORUP, True, False, False,
                        12, 12, 1),
                 lambda: self.assert_(self.mouseUp1Called and mainMouseUpCalled),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 70, 1),
                 lambda: self.assert_(self.mouseDown2Called and self.divMouseDownCalled and 
                        self.mouseOut1Called and not(self.obscuredMouseDownCalled)),
                 testInactiveDiv,
                 lambda: self.assert_(self.obscuredMouseDownCalled),
                 # Test if deactivation between mouse click and mouse out works.
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        70, 10, 1),
                 lambda: self.assert_(self.deactMouseOverCalled and not(self.deactMouseOverLate)
                        and not(self.neverCalledCalled)),
                 disableHandler,
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        10, 10, 1),
                 lambda: self.assert_(not(self.mouseDown1Called)),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        0, 64, 1),
                 lambda: self.assert_(not(self.tiltedMouseDownCalled)),
                 lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        0, 80, 1),
                 lambda: self.assert_(self.tiltedMouseDownCalled),
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
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                    100, 10, 1)
        def noCaptureEvent():
            global captureMouseDownCalled
            Helper = Player.getTestHelper()
            captureMouseDownCalled = False
            mainCaptureMouseDownCalled = False
            Player.getElementByID("img1").releaseEventCapture()
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                    100, 10, 1)
        global captureMouseDownCalled
        global mainCaptureMouseDownCalled
        Helper = Player.getTestHelper()
        self.start("eventcapture.avg",
                (lambda: Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
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
        testBrokenFile("filedoesntexist.avg")
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
        def checkRelPos():
            RelPos = Player.getElementByID("obscured").getRelPos((50,52))
            self.assert_(RelPos == (0, 0))
        self.start("avg.avg",
                (lambda: self.compareImage("testMove1", False),
                 moveit,
                 checkRelPos,
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
        def rotate():
            node = Player.getElementByID("img")
            node.x = 10
            node.y = 10
            Player.getElementByID("nestedavg").angle = 1.0
            Player.getElementByID("bkgd").angle = 1.0
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

                 rotate,
                 lambda: self.compareImage("testCropImage6", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropImage7", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropImage8", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropImage9", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropImage10", False),
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
        def rotate():
            node = Player.getElementByID("movie")
            node.x = 10
            node.y = 10
            Player.getElementByID("nestedavg").angle = 1.0
            Player.getElementByID("bkgd").angle = 1.0
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

                 rotate,
                 lambda: self.compareImage("testCropMovie6", False),
                 moveTLCrop,
                 lambda: self.compareImage("testCropMovie7", False),
                 moveBRCrop,
                 lambda: self.compareImage("testCropMovie8", False),
                 moveTLNegative,
                 lambda: self.compareImage("testCropMovie9", False),
                 moveBRGone,
                 lambda: self.compareImage("testCropMovie10", False),
                 Player.stop))

    def testWarp(self):
        def moveVertex():
            node = Player.getElementByID("testtiles")
            grid = node.getWarpedVertexCoords()
            grid[1][1] = (grid[1][1][0]+0.06, grid[1][1][1]+0.06)
            node.setWarpedVertexCoords(grid)
            node = Player.getElementByID("clogo1")
            grid = node.getWarpedVertexCoords()
            grid[0][0] = (grid[0][0][0]+0.06, grid[0][0][1]+0.06)
            node.setWarpedVertexCoords(grid)
        def flip():
            node = Player.getElementByID("testtiles")
            grid = node.getOrigVertexCoords()
            grid = [ [ (1-pos[0], pos[1]) for pos in line ] for line in grid]
            node.setWarpedVertexCoords(grid)
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
            oldwidth = node.width
            node.text = "blue" 
            self.assert_(node.width != oldwidth)
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
#                 changeText,
#                 changeHeight,
#                 deactivateText,
#                 lambda: self.compareImage("testWords2", True),
#                 activateText,
#                 changeFont,
#                 lambda: self.compareImage("testWords3", True),
#                 changeFont2,
#                 changeUnicodeText,
#                 lambda: self.compareImage("testWords4", True),
                 Player.stop))

    def testVideo(self):
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
        Player.setFakeFPS(25)
        self.start("video.avg",
                (lambda: self.compareImage("testVideo1", False),
                 lambda: Player.getElementByID("clogo2").play(),
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

    def testVideoSeek(self):
        def seek(frame):
            Player.getElementByID("clogo2").seekToFrame(frame)
        Player.setFakeFPS(25)
        self.start("video.avg",
                (lambda: Player.getElementByID("clogo2").play(),
                 lambda: seek(100),
                 lambda: self.compareImage("testVideoSeek1", False),
                 lambda: Player.getElementByID("clogo2").pause(),
                 lambda: seek(26),
                 lambda: self.delay,
                 lambda: self.compareImage("testVideoSeek2", False),
                 lambda: Player.getElementByID("clogo2").play(),
                 lambda: self.delay,
                 lambda: self.compareImage("testVideoSeek3", False),
                 Player.stop))
    
    def testVideoFPS(self):
        Player.setFakeFPS(25)
        self.start("videofps.avg",
                (lambda: Player.getElementByID("video").play(),
                 lambda: self.delay,
                 lambda: self.compareImage("testVideoFPS", False),
                 Player.stop))

    def testVideoEOF(self):
        def onEOF():
            Player.stop()
        def onNoEOF():
            self.assert_(False)
        Player.loadFile("video.avg")
        Player.setFakeFPS(25)
        video = Player.getElementByID("clogo1")
        video.play()
        video.setEOFCallback(onEOF)
        Player.setTimeout(10000, onNoEOF)
        Player.play()

    def testMediaDir(self):
        def setDir():
            Player.getElementByID("main").mediadir="."
        self.start("mediadir.avg",
                (lambda: Player.getElementByID("video").play(),
                 lambda: self.compareImage("mediadir1", False),
                 setDir,
                 lambda: Player.getElementByID("video").play(), 
                 lambda: self.compareImage("mediadir2", False),
                 Player.stop))

#    def testCamera(self):
#        def createCameraNode(deviceFile):
#            return Player.createNode("<camera id='camera1' width='640' height='480' "
#                    "source='v4l' pixelformat='YUYV422' "
#                    "capturewidth='640' captureheight='480' device="+deviceFile+
#                    " framerate='30'/>")
#        def findCamera():
#            node = createCameraNode("/dev/video0")
#            if node.getDriverName() != "vivi":
#                node = createCameraNode("/dev/video1")
#            if node.getDriverName() != "vivi":
#                print("Kernel camera test driver not found - skipping camera test.")
#                Player.stop()
#            else:
#                Player.getRootNode().appendChild(node)
#                node.play()
#
#        self.start("empty.avg",
#                (lambda: findCamera,
#                 lambda: self.compareImage("testCamera", False),
#                 Player.stop))

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
        Player.setFakeFPS(60)
        anim.init(Player)
        Player.loadFile("avg.avg")
        Player.setTimeout(1, onStart)
        Player.setFramerate(60)
        Player.play()

    def testContinuousAnim(self):
        def onStart():
            Player.setTimeout(10,startAnim)
            Player.setTimeout(100,lambda:self.compareImage("testContAnim1", False))
            Player.setTimeout(200,startAnim2)
            Player.setTimeout(400,lambda:self.compareImage("testContAnim2", False))
            Player.setTimeout(450,startAnim3)
            Player.setTimeout(700,lambda:self.compareImage("testContAnim3", False))
            Player.setTimeout(800,stopAnim)
            Player.setTimeout(900,lambda:self.compareImage("testContAnim4", False))
            Player.setTimeout(1000,Player.stop)
        def startAnim():
            node=Player.getElementByID("mainimg")
            self.anim=anim.ContinuousAnim(node,"angle",0,1,0)
        def startAnim2():
            node=Player.getElementByID("nestedimg1")
            self.anim2=anim.ContinuousAnim(node,"width",0,50,0)
        def startAnim3():
            node=Player.getElementByID("nestedimg2")
            self.anim3=anim.ContinuousAnim(node,"x",0,50,0)
        def stopAnim():
            self.anim.abort()
            self.anim2.abort()
            self.anim3.abort()

        Player.setFakeFPS(25)
        anim.init(Player)
        Player.loadFile("avg.avg")
        Player.setTimeout(1, onStart)
        Player.play()

    def testDraggable(self):
        def onDragStart(event):
            self.__dragStartCalled = True
        def onDragEnd(event):
            self.__dragEndCalled = True
        def startDrag():
            Helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False, 140, 40, 1)
        def move():
            Helper.fakeMouseEvent(avg.CURSORMOTION, True, False, False, 150, 50, 1)
        def stop():
            Helper.fakeMouseEvent(avg.CURSORUP, True, False, False, 140, 40, 1)
        self.__dragEndCalled = False
        self.__dragStartCalled = False
        Helper = Player.getTestHelper()    
        Player.loadFile("image.avg")
        draggable.init(avg, Player)
        dragger = draggable.Draggable(Player.getElementByID("testhue"),
                onDragStart, onDragEnd)
        dragger.enable()
        self.start(None,
                (startDrag,
                 lambda: self.assert_(self.__dragStartCalled),
                 move,
                 lambda: self.compareImage("testDraggable1", False),
                 stop,
                 lambda: self.assert_(self.__dragEndCalled),
                 lambda: self.compareImage("testDraggable2", False),
                 dragger.disable,
                 startDrag,
                 move,
                 lambda: self.compareImage("testDraggable2", False),
                 Player.stop))

    def testImgDynamics(self):
        def createImg():
            node = Player.createNode("<image href='rgb24-64x64.png'/>")
            node.id = "newImage"
            node.x = 10
            node.y = 20
            node.angle = 0.1
#            print node.toXML()
            rootNode = Player.getRootNode()
            rootNode.appendChild(node)
            exceptionRaised=False
            try:
                node.id = "bork"
            except RuntimeError:
                exceptionRaised=True
            self.assert_(exceptionRaised)
            self.assert_(rootNode.indexOf(Player.getElementByID("newImage")) == 0)
        def createImg2():
            node = Player.createNode("<image href='rgb24-64x64.png' id='newImage2'/>")
            Player.getRootNode().insertChild(node, 0)
        def reorderImg():
            Player.getRootNode().reorderChild(0, 1)
        def removeImgs():
            self.imgNode = Player.getElementByID("newImage")
            rootNode = Player.getRootNode()
            rootNode.removeChild( rootNode.indexOf(self.imgNode))
            rootNode.removeChild(0)
            self.assert_(Player.getElementByID("newImage") == None)
        def reAddImg():
            rootNode = Player.getRootNode()
            rootNode.appendChild(self.imgNode)
            self.imgNode = None
        Player.loadFile("empty.avg")
        createImg()
        Player.stop()
        self.setUpVideo()
        self.start("empty.avg",
                (createImg,
                 lambda: self.compareImage("testImgDynamics1", False),
                 createImg2,
                 lambda: self.compareImage("testImgDynamics2", False),
                 reorderImg,
                 lambda: self.compareImage("testImgDynamics3", False),
                 removeImgs,
                 lambda: self.compareImage("testImgDynamics4", False),
                 reAddImg,
                 lambda: self.compareImage("testImgDynamics5", False),
                 Player.stop))

    def testVideoDynamics(self):
        def createVideo():
            node = Player.createNode("<video id='newVideo' href='mpeg1-48x48.mpg'/>")
            Player.getRootNode().appendChild(node)
            node.play()
        def removeVideo():
            self.videoNode = Player.getElementByID("newVideo")
            rootNode = Player.getRootNode()
            rootNode.removeChild(rootNode.indexOf(self.videoNode))
        def reAddVideo():
            rootNode = Player.getRootNode()
            rootNode.appendChild(self.videoNode)
            exceptionRaised = False
            try:
                rootNode.appendChild(self.videoNode)
            except RuntimeError:
                exceptionRaised = True
            self.assert_(exceptionRaised)
            self.videoNode.play()
            self.videoNode = None
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
            Player.getRootNode().appendChild(node)
        def removeWords():
            self.wordsNode = Player.getElementByID("newWords")
            rootNode = Player.getRootNode()
            rootNode.removeChild(rootNode.indexOf(self.wordsNode))
        def reAddWords():
            rootNode = Player.getRootNode()
            rootNode.appendChild(self.wordsNode)
            self.wordsNode.text='test2'
            self.wordsNode = None
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
            node = Player.createNode("<camera id='newCamera' source='firewire' device='/dev/video1394/0' capturewidth='640' captureheight='480' pixelformat='MONO8' framerate='15'/>")
            Player.getRootNode().appendChild(node)
        def removeCamera():
            self.cameraNode = Player.getElementByID("newCamera")
            rootNode = Player.getRootNode()
            rootNode.removeChild(rootNode.indexOf(self.cameraNode))
        def reAddCamera():
            Player.getRootNode().appendChild(self.cameraNode)
            self.cameraNode = None
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
            Player.getRootNode().appendChild(node)
        def removePano():
            self.panoNode = Player.getElementByID("newPano")
            rootNode = Player.getRootNode()
            rootNode.removeChild(rootNode.indexOf(self.panoNode))
        def reAddPano():
            Player.getRootNode().appendChild(self.panoNode)
            self.panoNode = None
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
            Player.getRootNode().appendChild(node)
        def removeDiv():
            self.divNode = Player.getElementByID("newDiv")
            rootNode = Player.getRootNode()
            rootNode.removeChild(rootNode.indexOf(self.divNode))
        def reAddDiv():
            node = Player.createNode("<image id='img2' href='rgb24-64x64.png' x='64'/>")
            self.divNode.appendChild(node)
            Player.getRootNode().appendChild(self.divNode)
            self.divNode = None
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
            
def playerTestSuite(bpp):
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
    suite.addTest(PlayerTestCase("testImage", bpp))
    suite.addTest(PlayerTestCase("testRotate", bpp))
    suite.addTest(PlayerTestCase("testRotate2", bpp))
    suite.addTest(PlayerTestCase("testRotate3", bpp))
    suite.addTest(PlayerTestCase("testError", bpp))
    suite.addTest(PlayerTestCase("testExceptionInTimeout", bpp))
    suite.addTest(PlayerTestCase("testInvalidImageFilename", bpp))
    suite.addTest(PlayerTestCase("testInvalidVideoFilename", bpp))
    suite.addTest(PlayerTestCase("testEvents", bpp))
    suite.addTest(PlayerTestCase("testEventCapture", bpp))
    suite.addTest(PlayerTestCase("testTimeouts", bpp))
    suite.addTest(PlayerTestCase("testEventErr", bpp))
    suite.addTest(PlayerTestCase("testHugeImage", bpp))
    suite.addTest(PlayerTestCase("testPanoImage", bpp))
    suite.addTest(PlayerTestCase("testBroken", bpp))
    suite.addTest(PlayerTestCase("testMove", bpp))
    suite.addTest(PlayerTestCase("testBlend", bpp))
    suite.addTest(PlayerTestCase("testCropImage", bpp))
    suite.addTest(PlayerTestCase("testCropMovie", bpp))
    suite.addTest(PlayerTestCase("testWords", bpp))
    suite.addTest(PlayerTestCase("testVideo", bpp))
    suite.addTest(PlayerTestCase("testVideoSeek", bpp))
    suite.addTest(PlayerTestCase("testVideoFPS", bpp))
    suite.addTest(PlayerTestCase("testVideoEOF", bpp))
    suite.addTest(PlayerTestCase("testMediaDir", bpp))
    suite.addTest(PlayerTestCase("testWarp", bpp))
    suite.addTest(PlayerTestCase("testAnim", bpp))
    suite.addTest(PlayerTestCase("testContinuousAnim", bpp))
    suite.addTest(PlayerTestCase("testDraggable", bpp))
    suite.addTest(PlayerTestCase("testImgDynamics", bpp))
    suite.addTest(PlayerTestCase("testVideoDynamics", bpp))
    suite.addTest(PlayerTestCase("testWordsDynamics", bpp))
    suite.addTest(PlayerTestCase("testPanoDynamics", bpp))
    suite.addTest(PlayerTestCase("testCameraDynamics", bpp))
    suite.addTest(PlayerTestCase("testDivDynamics", bpp))
    return suite

def completeTestSuite(bpp):
    suite = unittest.TestSuite()
    suite.addTest(LoggerTestCase("test"))
    suite.addTest(playerTestSuite(bpp))
    return suite

def runConsoleTest():
    Player = avg.Player()
    Player.loadFile("video.avg")

def getBoolParam(paramIndex):
    param = sys.argv[paramIndex].upper()
    if param == "TRUE":
        return True
    elif param == "FALSE":
        return False
    else:
        print "Parameter "+paramIndex+" must be 'True' or 'False'"

if os.getenv("AVG_CONSOLE_TEST"):
    runConsoleTest()
else:
    if len(sys.argv) == 1:
        bpp = 24
        customOGLOptions = False
    elif len(sys.argv) == 2 or len(sys.argv) == 5:
        bpp = int(sys.argv[1])
        if (len(sys.argv) == 5):
            customOGLOptions = True
            UsePOW2Textures = getBoolParam(2)
            s = sys.argv[3]
            if s == "shader":
                YCbCrMode = avg.shader
            elif s == "apple":
                YCbCrMode = avg.apple
            elif s == "mesa":
                YCbCrMode = avg.mesa
            elif s == "none":
                YCbCrMode = avg.none
            else:
                print "Third parameter must be shader, apple, mesa or none"
                sys.exit(1)
            UsePixelBuffers = getBoolParam(4)
        else:
            customOGLOptions = False
    else:
        print "Usage: Test.py [<bpp>"
        print "               [<UsePOW2Textures> <YCbCrMode> <UsePixelBuffers>]]"
        sys.exit(1)

    Player = avg.Player()
    runner = unittest.TextTestRunner()
    rc = runner.run(completeTestSuite(bpp))
    if rc.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)

