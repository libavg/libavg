#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, syslog
# TODO: set this path via configure or something similar.
sys.path.append('/usr/local/lib/python2.3/site-packages/libavg')
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
import avg
import time
import anim

class LoggerTestCase(unittest.TestCase):
    def test(self):
        self.Log = avg.Logger.get()
        self.Log.setCategories(self.Log.APP |
                  self.Log.WARNING | 
                  self.Log.PROFILE |
#                  self.Log.PROFILE_LATEFRAMES |
                  self.Log.CONFIG |
                  self.Log.MEMORY | 
#                  self.Log.BLTS    |
                  self.Log.EVENTS
                  )
        self.Log.setFileDest("testavg.log")
        self.Log.trace(self.Log.APP, "Test file log entry.")
        self.Log.setSyslogDest(syslog.LOG_USER, syslog.LOG_CONS)
        self.Log.trace(self.Log.APP, "Test syslog entry.")
        self.Log.setConsoleDest()

class ParPortTestCase(unittest.TestCase):
    def test(self):
        def setAllLines(val):
            self.ParPort.setControlLine(avg.CONTROL_STROBE, val)
            self.ParPort.setControlLine(avg.CONTROL_AUTOFD, val)
            self.ParPort.setControlLine(avg.CONTROL_SELECT, val)
            self.ParPort.setControlLine(avg.CONTROL_INIT, val)
        self.ParPort = avg.ParPort()
        self.ParPort.init("")
        setAllLines(1)
        time.sleep(0.5)
        setAllLines(0)
        print self.ParPort.getStatusLine(avg.STATUS_ERROR)
        print self.ParPort.getStatusLine(avg.STATUS_SELECT)
        print self.ParPort.getStatusLine(avg.STATUS_PAPEROUT)
        print self.ParPort.getStatusLine(avg.STATUS_ACK)
        print self.ParPort.getStatusLine(avg.STATUS_BUSY)
        self.ParPort.setDataLines(avg.PARPORTDATA0 | avg.PARPORTDATA1)
        time.sleep(0.2)
        self.ParPort.setDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.2)
        self.ParPort.clearDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.2)
        self.ParPort.clearDataLines(avg.PARPORTDATA0 | avg.PARPORTDATA1)

class ConradRelaisTestCase(unittest.TestCase):
    def test(self):
        ConradRelais = avg.ConradRelais(Player, 0)
        print ConradRelais.getNumCards()
        for i in range(6):
            ConradRelais.set(0, i, 1)

class NodeTestCase(unittest.TestCase):
    def testAttributes(self):
        self.Image = avg.Image()
        assert self.Image.id == ""
        self.Image.x = 10
        self.Image.x += 1
        assert self.Image.x == 11

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


def mainMouseUp():
    print "mainMouseUp"
#    dumpMouseEvent()

def mainMouseDown():
    print "mainMouseDown"
#    dumpMouseEvent()

def onMouseMove():
    print "onMouseMove"
#    dumpMouseEvent()

def onMouseUp():
    print "onMouseUp"
#    dumpMouseEvent()

def onMouseOver():
    print "onMouseOver"
    dumpMouseEvent()

def onMouseOut():
    print "onMouseOut"
    dumpMouseEvent()

def onMouseDown():
    print "onMouseDown"
    Player.getElementByID("mouseover1").active=0
    Player.getElementByID("rightdiv").active=0
    dumpMouseEvent()

def onErrMouseOver():
    undefinedFunction()

class PlayerTestCase(unittest.TestCase):
    def __init__(self, testFuncName, engine, bpp):
        self.__engine = engine
        self.__bpp = bpp
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
    def setUp(self):
        Player.setDisplayEngine(self.__engine)
        Player.setResolution(0, 0, 0, self.__bpp)
        print "-------- ", self.__testFuncName, " --------"
    def playAVG(self, fileName):
        Player.loadFile(fileName)
        Player.setTimeout(100, lambda: Player.screenshot("test.png"))
        Player.setTimeout(150, lambda: Player.screenshot("test1.png"))
        Player.setTimeout(200, lambda: self.assert_(Player.isPlaying() == 1)) 
        Player.setTimeout(250, Player.stop)
        Player.setVBlankFramerate(1)
        Player.play()
        
    def testImage(self):
        def loadNewFile():
            Player.getElementByID("test").href = "1x1_schachbrett.png"
            Player.getElementByID("testtiles").href = "freidrehen.jpg"
            Player.getElementByID("testhue").href = "freidrehen.jpg"
        self.assert_(Player.isPlaying() == 0)
        Player.loadFile("image.avg")
        Player.setTimeout(1000, loadNewFile)
        Player.setTimeout(2000, Player.stop)
        Player.setVBlankFramerate(1)
        Player.play()
        self.assert_(Player.isPlaying() == 0)
    def testError(self):
        Player.setTimeout(100, lambda: undefinedFunction)
        try:
            self.playAVG("image.avg")
        except NameError:
            self.assert_(1)
        else:
            self.assert_(0)
    def testEvents(self):
        def getMouseState():
            Event = Player.getMouseState()
            print(str(Event.x)+","+str(Event.y))
        Player.loadFile("events.avg")
        Player.setTimeout(100, getMouseState);
        Player.setTimeout(200, Player.stop)
        Player.setVBlankFramerate(2)
        Player.play()
    def testEventErr(self):
        Player.loadFile("errevent.avg")
        Player.setTimeout(1000, Player.stop)
        try:
            Player.play()
        except NameError:
            self.assert_(1)
    def createNodes(self):
        node=Player.createNode("<image href='rgb24.png'/>")
        node.x = 10
        node.y = 20
        node.z = 2
        node.opacity = 0.333
        node.angle = 0.1
        node.blendmode = "add"
#        print node.toXML()
        self.rootNode.addChild(node)
#        nodeCopy = node
#        self.rootNode.addChild(nodeCopy)
        node = Player.createNode("<video href='test.m1v'/>")
        self.rootNode.addChild(node)
        node = Player.createNode("<words text='Lorem ipsum dolor'/>")
        self.rootNode.addChild(node)
        node.size = 18
        node.font = "times new roman"
        node.parawidth = 200
        node = Player.createNode("<div><image href='rgb24.png'/></div>")
        node.getChild(0).x=10
        node.x=10
        self.rootNode.addChild(node)
    def deleteNodes(self):
        for i in range(self.rootNode.getNumChildren()-1,0):
#            print ("Deleting node #"+i);
            self.rootNode.removeChild(i)
#    def testDynamics(self):
#        Player.loadFile("image.avg")
#        self.rootNode = Player.getRootNode()
#        print self.rootNode.indexOf(Player.getElementByID("mainimg"));
#        print self.rootNode.indexOf(Player.getElementByID("testtiles"));
#        self.createNodes()
#        Player.setTimeout(250, self.deleteNodes)
#        Player.setTimeout(500, self.createNodes)
#        Player.play()
    def testHugeImage(self):
        def moveImage():
            Player.getElementByID("mainimg").x -= 50
        Player.loadFile("hugeimage.avg")
        timerid = Player.setInterval(10, moveImage)
        Player.setTimeout(1000, Player.stop)
        Player.setVBlankFramerate(1)
        Player.play()
    def testPanoImage(self):
        self.playAVG("panoimage.avg")
    def testBroken(self):
        Player.loadFile("noxml.avg")
        Player.loadFile("noavg.avg")
    def testExcl(self):
        def setExcl(i):
            node = Player.getElementByID("switch")
            node.activechild = i
        def activateExcl(b):
            node = Player.getElementByID("switch")
            node.active = b
        def move():
            node = Player.getElementByID("div")
            node.x += 1
        Player.loadFile("excl.avg")
        Player.setTimeout(300, lambda : setExcl(0))
        Player.setTimeout(600, lambda : setExcl(1))
        Player.setTimeout(900, lambda : setExcl(2))
        Player.setTimeout(1200, lambda : setExcl(3))
        Player.setTimeout(1500, lambda : activateExcl(0))
        Player.setTimeout(1500, lambda : activateExcl(1))
        Player.setInterval(10, move)
        Player.setTimeout(2000, Player.stop)
        Player.play()
    def testAnimation(self):
        def moveit():
            node = Player.getElementByID("nestedimg1")
            node.x += 1
            node.opacity -= 0.01
        def moveit2():
            node = Player.getElementByID("nestedavg")
            node.x += 1
        Player.loadFile("avg.avg")
        node = Player.getElementByID("nestedimg1")
        print("    Node id: "+node.id)
        Player.setInterval(10, moveit)
        Player.setInterval(10, moveit2)
        Player.setTimeout(2000, Player.stop)
        Player.play()
    def testBlend(self):
        def moveBlended():
            for i in range(4):
                node = Player.getElementByID("blend"+str(i))
                node.x += 1
        Player.loadFile("blend.avg")
        Player.setInterval(10, moveBlended)
        Player.setTimeout(2000, Player.stop)
        Player.play()
    def testCrop(self):
        def cropTL():
            node = Player.getElementByID("img")
            node.x -= 2
            node.y -= 2
            if node.x < -250:
                Player.clearInterval(self.cropInterval)
                self.cropInterval = Player.setInterval(10, cropBR)
        def cropBR():
            node = Player.getElementByID("img")
            if node.x < 0:
                node.x = 100
                node.y = 50
            node.x +=2
            node.y +=2
            if node.x > 700:
                Player.clearInterval(self.cropInterval)
                Player.stop()
        def goneTL():
            node = Player.getElementByID("img")
            node.x = -250
            node.y = -250
        def goneBR():
            node = Player.getElementByID("img")
            node.x = 750
            node.y = 650
        Player.loadFile("crop.avg")
        self.cropInterval = Player.setInterval(10, cropTL)
        Player.getElementByID("img").play()
        Player.play()
        self.setUp()
        Player.loadFile("crop2.avg")
        self.cropInterval = Player.setInterval(10, cropTL)
        Player.play()
    def testUnicode(self):
        Player.loadFile("unicode.avg")
        Player.getElementByID("dynamictext").text = "Arabic nonsense: ﯿﭗ"
        Player.setTimeout(1000, Player.stop)
        Player.play()
    def testWarp(self):
        def moveVertex():
            node = Player.getElementByID("testtiles")
            pos = node.getWarpedVertexCoord(1,1)
            pos.x += 0.002
            pos.y += 0.002
            node.setWarpedVertexCoord(1,1,pos)
            node = Player.getElementByID("clogo1")
            pos = node.getWarpedVertexCoord(0,0)
            pos.x += 0.002
            pos.y += 0.002
            node.setWarpedVertexCoord(0,0,pos)
        Player.loadFile("video.avg")
        Player.getElementByID("clogo1").play()
        node = Player.getElementByID("testtiles")
        print("Vertices: "+str(node.getNumVerticesX())+"x"
                +str(node.getNumVerticesY()))
        Player.setInterval(10, moveVertex)
        Player.setTimeout(5000, Player.stop)
        Player.play()  
        
        
class WordsTestCase(unittest.TestCase):
    def __init__(self, testFuncName, engine, bpp):
        self.__engine = engine
        self.__bpp = bpp;
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
    def setUp(self):
        Player.setDisplayEngine(self.__engine)
        Player.setResolution(0, 0, 0, self.__bpp)
        print "-------- ", self.__testFuncName, " --------"
    def test(self):
        def textInterval():
            node = Player.getElementByID("cbasetext")
            self.delay += 1
            if self.delay == 10:
                self.numChars += 1
                self.delay = 0
            str = "hello c-base"[:self.numChars]
            node.text = str
            node.x += 1
        def changeTextHeight():
            node = Player.getElementByID("cbasetext")
            node.height = 50
            l = node.x
            t = node.y
            w = node.width
            h = node.height
            print "Pos: (",l,",",t,",",w,",",h,")"
        def changeColor():
            node = Player.getElementByID("cbasetext")
            node.color = "404080"
        def activateText():
            Player.getElementByID('cbasetext').active = 1
        def deactivateText():
            Player.getElementByID('cbasetext').active = 0
        def changeFont():
            node = Player.getElementByID("cbasetext")
            node.font = "Lucida Console"
            node.size = 50
        def changeFont2():
            node = Player.getElementByID("cbasetext")
            node.size = 30
        self.delay = 0
        self.numChars = 0
        Player.loadFile("text.avg")
        node = Player.getElementByID("paramarkup")
        timerid = Player.setInterval(10, textInterval)
        Player.setTimeout(300, changeTextHeight)
        Player.setTimeout(600, changeColor)
        Player.setTimeout(900, deactivateText)
        Player.setTimeout(1200, activateText)
        Player.setTimeout(1500, changeFont)
        Player.setTimeout(1800, changeFont2)
        Player.setTimeout(2000, Player.stop)
        Player.play()

class VideoTestCase(unittest.TestCase):
    def __init__(self, testFuncName, engine, bpp):
        self.__engine = engine
        self.__bpp = bpp;
        self.__testFuncName = testFuncName
        unittest.TestCase.__init__(self, testFuncName)
        print "-------- ", self.__testFuncName, " --------"
    def setUp(self):
        Player.setDisplayEngine(self.__engine)
        Player.setResolution(0, 0, 0, self.__bpp)
    def test(self):
        def playVideo(nodeName):
            node = Player.getElementByID(nodeName)
            node.play()
            if nodeName == "clogo2":
                node.seekToFrame(25)
        def playclogo():
            playVideo("clogo")
        def playclogo1():
            playVideo("clogo1")
        def playclogo2():
            playVideo("clogo2")
        def interval():
            node = Player.getElementByID("clogo2")
            node.x += 1
            self.curFrame -= 1
            if self.curFrame == 0:
                self.curFrame = 200
            node.seekToFrame(self.curFrame)
        def activateclogo2():
            Player.getElementByID('clogo2').active=1
        def deactivateclogo2():
            Player.getElementByID('clogo2').active=0
        def pause():
            node = Player.getElementByID("clogo")
            node.pause()
        def stop():
            node = Player.getElementByID("clogo")
            node.stop()
        def newHRef():
            node = Player.getElementByID("clogo2")
            node.href = "test.m1v"
            node.play()
        self.curFrame = 200
        Player.loadFile("video.avg")
        playVideo("clogo")
        Player.getElementByID("clogo2").pause()
        Player.setTimeout(500, newHRef)
        Player.setTimeout(1000, playclogo1)
        self.timerid = Player.setInterval(10, interval)
        Player.setTimeout(1500, playclogo2)
        Player.setTimeout(2000, pause)
        Player.setTimeout(2500, playclogo)
        Player.setTimeout(2500, deactivateclogo2)
        Player.setTimeout(3000, activateclogo2)
        Player.setTimeout(3000, stop)
        Player.setTimeout(3500, playclogo)
        Player.setTimeout(5000, Player.stop)
        Player.setFramerate(25)
        Player.play()

class AnimTestCase(unittest.TestCase):
    def __init__(self, testFuncName, engine, bpp):
        self.__engine = engine
        self.__bpp = bpp
        self.__testFuncName = testFuncName
        self.__animStopped = 0
        unittest.TestCase.__init__(self, testFuncName)
    def setUp(self):
        Player.setDisplayEngine(self.__engine)
        Player.setResolution(0, 0, 0, self.__bpp)
    def test(self):
        def startAnim():
            def onStop():
                self.__animStopped = 1
            anim.fadeOut(Player.getElementByID("nestedimg2"), 1000)
            Player.getElementByID("nestedimg1").opacity = 0
            anim.fadeIn(Player.getElementByID("nestedimg1"), 1000, 1)
            anim.LinearAnim(Player.getElementByID("nestedimg1"), "x", 
                    1000, 0, 100, 0, onStop)
        def startSplineAnim():
            anim.SplineAnim(Player.getElementByID("mainimg"), "x", 
                    2000, 100, -400, 10, 0, 0, None)
            anim.SplineAnim(Player.getElementByID("mainimg"), "y", 
                    2000, 100, 0, 10, -400, 1, None)
        anim.init(Player)
        Player.loadFile("avg.avg")
        Player.setTimeout(4200, Player.stop)
        Player.setTimeout(10, startAnim)
        Player.setTimeout(1100, startSplineAnim)
        Player.setTimeout(1500, lambda: self.assert_(self.__animStopped == 1))
        Player.setVBlankFramerate(1)
        Player.play()


def playerTestSuite(engine, bpp):
    suite = unittest.TestSuite()
 
    suite.addTest(LoggerTestCase("test"))
    if sys.platform != "darwin":
        suite.addTest(ParPortTestCase("test"))
    suite.addTest(ConradRelaisTestCase("test"))
    suite.addTest(NodeTestCase("testAttributes"))
    suite.addTest(PlayerTestCase("testImage", engine, bpp))
    suite.addTest(PlayerTestCase("testError", engine, bpp))
    suite.addTest(PlayerTestCase("testEvents", engine, bpp))
    suite.addTest(PlayerTestCase("testEventErr", engine, bpp))
#    suite.addTest(PlayerTestCase("testDynamics", engine, bpp))
    suite.addTest(PlayerTestCase("testHugeImage", engine, bpp))
    suite.addTest(PlayerTestCase("testBroken", engine, bpp))
    suite.addTest(PlayerTestCase("testExcl", engine, bpp))
    suite.addTest(PlayerTestCase("testAnimation", engine, bpp))
    suite.addTest(PlayerTestCase("testBlend", engine, bpp))
    suite.addTest(PlayerTestCase("testCrop", engine, bpp))
    suite.addTest(PlayerTestCase("testUnicode", engine, bpp))
    suite.addTest(WordsTestCase("test", engine, bpp))
    suite.addTest(VideoTestCase("test", engine, bpp))
    if engine == avg.OGL:
        suite.addTest(PlayerTestCase("testPanoImage", engine, bpp))
        suite.addTest(PlayerTestCase("testWarp", engine, bpp))
    suite.addTest(AnimTestCase("test", engine, bpp))
    return suite

Player = avg.Player()
runner = unittest.TextTestRunner()

if len(sys.argv) != 3:
    print "Usage: Test.py <display engine> <bpp>"
else:
    if sys.argv[1] == "OGL":
        engine = avg.OGL
    elif sys.argv[1] == "DFB":
        engine = avg.DFB
    else:
        print "First parameter must be OGL or DFB"
    bpp = int(sys.argv[2])

#    runner.run(LoggerTestCase("test"))
#    runner.run(VideoTestCase("test", engine, bpp))
    runner.run(playerTestSuite(engine, bpp))

