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
        time.sleep(0.5)
        self.ParPort.setDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.5)
        self.ParPort.clearDataLines(avg.PARPORTDATA2 | avg.PARPORTDATA3)
        time.sleep(0.5)
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
        node.size = 30;
    def testWords(self):
        self.delay = 0;
        self.numChars = 0;
        Player.loadFile("text.avg")
        node = Player.getElementByID("paramarkup")
        timerid = Player.setInterval(10, self.textInterval)
        Player.setTimeout(1000, self.changeTextHeight)
        Player.setTimeout(2000, self.changeColor)
        Player.setTimeout(2300, self.deactivateText)
        Player.setTimeout(2600, self.activateText)
        Player.setTimeout(3000, self.changeFont)
        Player.setTimeout(4000, self.changeFont2)
        Player.setTimeout(10000, Player.stop)
        Player.play(25);
        
    def testHugeImage(self):
        self.playAVG("hugeimage.avg")
    def testPanoImage(self):
        self.playAVG("panoimage.avg")

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
    suite.addTest(PlayerTestCase("testWords"))
    return suite

runner = unittest.TextTestRunner()
runner.run(playerTestSuite())

Player.loadFile("noxml.avg")
#Player.loadFile("noavg.avg")

Player.loadFile("crop2.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("excl.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("video.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("camera.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

