#!/usr/bin/python

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg

Log = avg.Logger.get()
Log.setCategories(Log.APP | Log.WARNING | Log.PROFILE)

Image = avg.Image()
print(Image)
print(Image.ID)
print(Image.href)
Image.x = 10
print(Image.x)
Player = avg.Player()

def Stop():
    Player.stop()

def keyUp():
    print "keyUp"

def keyDown():
    print "keyDown"

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

def DoScreenshot():
    Player.screenshot("test.png")

def PlayAVG(fileName):
    Player.loadFile(fileName)
    Player.setTimeout(100, DoScreenshot)
    Player.setTimeout(200, Stop)
    Player.play(30)

PlayAVG("test/image.avg")

Player.loadFile("test/events.avg")
Player.play(30)

Player.loadFile("test/hugeimage.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("test/panoimage.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("test/noxml.avg")
#Player.loadFile("test/noavg.avg")

Player.loadFile("test/crop2.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("test/excl.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("test/video.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("test/text.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

Player.loadFile("test/camera.avg")
Player.setTimeout(1000, Stop)
Player.play(30)

