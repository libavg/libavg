#!/usr/bin/python

import unittest

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg
import time

class LoggerTestCase(unittest.TestCase):
    def test(self):
        self.Log = avg.Logger.get()
        self.Log.setCategories(self.Log.APP |
                  self.Log.WARNING | 
                  self.Log.PROFILE |
                  self.Log.PROFILE_LATEFRAMES |
                  self.Log.CONFIG |
#                  self.Log.MEMORY  |
#                  self.Log.BLTS    |
                  self.Log.EVENTS)
        self.Log.trace(self.Log.APP, "Test JS log entry.")

class ParPortTestCase(unittest.TestCase):
    def setAllLines(self, val):
        self.ParPort.setControlLine(avg.CONTROL_STROBE, val)
        self.ParPort.setControlLine(avg.CONTROL_AUTOFD, val)
        self.ParPort.setControlLine(avg.CONTROL_SELECT, val)
        self.ParPort.setControlLine(avg.CONTROL_INIT, val)
    def test(self):
        self.ParPort = avg.ParPort()
        self.ParPort.init("")
        self.setAllLines(1)
        time.sleep(0.5)
        self.setAllLines(0)
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

class NodeTestCase(unittest.TestCase):
    def testAttributes(self):
        self.Image = avg.Image()
        assert self.Image.id == ""
        self.Image.x = 10
        self.Image.x += 1
        assert self.Image.x == 11

Player = avg.Player()

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

def mainMouseUp():
    print "mainMouseUp"

def mainMouseDown():
    print "mainMouseDown"

def onMouseMove():
    print "onMouseMove"

def onMouseUp():
    print "onMouseUp"

def onMouseOver():
    print "onMouseOver"

def onMouseOut():
    print "onMouseOut"

def onMouseDown():
    print "onMouseDown"

class PlayerTestCase(unittest.TestCase):
    def Stop(self):
        Player.stop()
    def DoScreenshot(self):
        Player.screenshot("test.png")
    def playAVG(self, fileName):
        Player.loadFile(fileName)
        Player.setTimeout(100, self.DoScreenshot)
        Player.setTimeout(200, self.Stop)
        Player.play(30)
        
    def testImage(self):
        self.playAVG("image.avg")
        
    def testEvents(self):
        Player.loadFile("events.avg")
        Player.setTimeout(200, Player.stop)
        Player.play(30)
        
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
        print 1
#        nodeCopy = node
#        self.rootNode.addChild(nodeCopy)
        node = Player.createNode("<video href='test.m1v'/>")
        print 2
        self.rootNode.addChild(node)
        print 3
        node = Player.createNode("<words text='Lorem ipsum dolor'/>")
        self.rootNode.addChild(node)
        print 4
        node.size = 18
        node.font = "times new roman"
        node.parawidth = 200
        node = Player.createNode("<div><image href='rgb24.png'/></div>")
        node.getChild(0).x=10
        node.x=10
        self.rootNode.addChild(node)
        print 5
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
#        Player.play(30)
    def moveImage(self):
        Player.getElementByID("mainimg").x -= 50
    def testHugeImage(self):
        Player.loadFile("hugeimage.avg")
        timerid = Player.setInterval(10, self.moveImage)
        Player.setTimeout(1000, Player.stop)
        Player.play(25)
    def testPanoImage(self):
        self.playAVG("panoimage.avg")
    def testBroken(self):
        Player.loadFile("noxml.avg")
        Player.loadFile("noavg.avg")
    def setExcl(self, i):
        node = Player.getElementByID("switch")
        node.activechild = i
    def activateExcl(self, b):
        node = Player.getElementByID("switch")
        node.active = b
    def testExcl(self):
        Player.loadFile("excl.avg")
        Player.setTimeout(300, lambda : self.setExcl(0))
        Player.setTimeout(600, lambda : self.setExcl(1))
        Player.setTimeout(900, lambda : self.setExcl(2))
        Player.setTimeout(1200, lambda : self.setExcl(3))
        Player.setTimeout(1500, lambda : self.activateExcl(0))
        Player.setTimeout(1500, lambda : self.activateExcl(1))
        Player.setTimeout(2000, Player.stop)
        Player.play(30)
    def moveit(self):
        node = Player.getElementByID("nestedimg1")
        node.x += 1
        node.opacity -= 0.01
    def testAnimation(self):
        Player.loadFile("avg.avg")
        node = Player.getElementByID("nestedimg1")
        print("    Node id: "+node.id)
        Player.setInterval(10, self.moveit)
        Player.setTimeout(2000, Player.stop)
        Player.play(30)
    def moveBlended(self):
        for i in range(4):
            node = Player.getElementByID("blend"+str(i))
            node.x += 1
    def testBlend(self):
        Player.loadFile("blend.avg")
        Player.setInterval(10, self.moveBlended)
        Player.setTimeout(2000, Player.stop)
        Player.play(30)
    def cropTL(self):
        node = Player.getElementByID("img")
        node.x -= 2
        node.y -= 2
        if node.x < -250:
            Player.clearInterval(self.cropInterval)
            self.cropInterval = Player.setInterval(10, self.cropBR)
    def cropBR(self):
        node = Player.getElementByID("img")
        if node.x < 0:
            node.x = 100
            node.y = 50
        node.x +=2
        node.y +=2
        if node.x > 700:
            Player.clearInterval(self.cropInterval)
            Player.stop()
    def goneTL(self):
        node = Player.getElementByID("img")
        node.x = -250
        node.y = -250
    def goneBR(self):
        node = Player.getElementByID("img")
        node.x = 750
        node.y = 650
    def testCrop(self):
        Player.loadFile("crop.avg")
        self.cropInterval = Player.setInterval(10, self.cropTL)
        Player.getElementByID("img").play()
        Player.play(30)
        Player.loadFile("crop2.avg")
        self.cropInterval = Player.setInterval(10, self.cropTL)
        Player.play(60)
    def moveVertex(self):
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
    def testWarp(self):
        Player.loadFile("video.avg")
        node = Player.getElementByID("testtiles")
        print("Vertices: "+str(node.getNumVerticesX())+"x"
                +str(node.getNumVerticesY()))
        Player.setInterval(10, self.moveVertex)
        Player.setTimeout(5000, Player.stop)
        Player.play(30)  
        
        
class WordsTestCase(unittest.TestCase):
    def textInterval(self):
        node = Player.getElementByID("cbasetext")
        self.delay += 1
        if self.delay == 10:
            self.numChars += 1
            self.delay = 0
        str = "hello c-base"[:self.numChars]
        node.text = str
        node.x += 1
    def changeTextHeight(self):
        node = Player.getElementByID("cbasetext")
        node.height = 50
        l = node.x
        t = node.y
        w = node.width
        h = node.height
        print "Pos: (",l,",",t,",",w,",",h,")"
    def changeColor(self):
        node = Player.getElementByID("cbasetext")
        node.color = "404080"
    def activateText(self):
        Player.getElementByID('cbasetext').active = 1
    def deactivateText(self):
        Player.getElementByID('cbasetext').active = 0
    def changeFont(self):
        node = Player.getElementByID("cbasetext")
        node.font = "Lucida Console"
        node.size = 50
    def changeFont2(self):
        node = Player.getElementByID("cbasetext")
        node.size = 30
    def test(self):
        self.delay = 0
        self.numChars = 0
        Player.loadFile("text.avg")
        node = Player.getElementByID("paramarkup")
        timerid = Player.setInterval(10, self.textInterval)
        Player.setTimeout(300, self.changeTextHeight)
        Player.setTimeout(600, self.changeColor)
        Player.setTimeout(900, self.deactivateText)
        Player.setTimeout(1200, self.activateText)
        Player.setTimeout(1500, self.changeFont)
        Player.setTimeout(1800, self.changeFont2)
        Player.setTimeout(2000, Player.stop)
        Player.play(25)

class VideoTestCase(unittest.TestCase):
    def playVideo(self, nodeName):
        node = Player.getElementByID(nodeName)
        node.play()
        if nodeName == "clogo2":
            node.seekToFrame(25)
    def playclogo(self):
        self.playVideo("clogo")
    def playclogo1(self):
        self.playVideo("clogo1")
    def playclogo2(self):
        self.playVideo("clogo2")
    def interval(self):
        node = Player.getElementByID("clogo2")
        node.x += 1
        self.curFrame -= 1
        if self.curFrame == 0:
            self.curFrame = 200
        node.seekToFrame(self.curFrame)
    def activateclogo2(self):
        Player.getElementByID('clogo2').active=1
    def deactivateclogo2(self):
        Player.getElementByID('clogo2').active=0
    def pause(self):
        node = Player.getElementByID("clogo")
        node.pause()
    def stop(self):
        node = Player.getElementByID("clogo")
        node.stop()
    def test(self):
        self.curFrame = 200
        Player.loadFile("video.avg")
        self.playVideo("clogo")
        Player.setTimeout(1000, self.playclogo1)
        self.timerid = Player.setInterval(10, self.interval)
        Player.setTimeout(1500, self.playclogo2)
        Player.setTimeout(2000, self.pause)
        Player.setTimeout(2500, self.playclogo)
        Player.setTimeout(2500, self.deactivateclogo2)
        Player.setTimeout(3000, self.activateclogo2)
        Player.setTimeout(3000, self.stop)
        Player.setTimeout(3500, self.playclogo)
        Player.setTimeout(5000, Player.stop)
        Player.play(25)

def playerTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(LoggerTestCase("test"))
    suite.addTest(ParPortTestCase("test"))
    suite.addTest(NodeTestCase("testAttributes"))
    suite.addTest(PlayerTestCase("testImage"))
    suite.addTest(PlayerTestCase("testEvents"))
#    suite.addTest(PlayerTestCase("testDynamics"))
    suite.addTest(PlayerTestCase("testHugeImage"))
    suite.addTest(PlayerTestCase("testPanoImage"))
    suite.addTest(PlayerTestCase("testBroken"))
    suite.addTest(PlayerTestCase("testExcl"))
    suite.addTest(PlayerTestCase("testAnimation"))
    suite.addTest(PlayerTestCase("testBlend"))
    suite.addTest(PlayerTestCase("testCrop"))
    suite.addTest(PlayerTestCase("testWarp"))
    suite.addTest(WordsTestCase("test"))
    suite.addTest(VideoTestCase("test"))
    return suite

runner = unittest.TextTestRunner()
runner.run(playerTestSuite())

