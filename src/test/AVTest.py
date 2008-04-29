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

from testcase import *


class AVTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)
    def testEOF(self, node):
        def onEOF():
            Player.stop()
        def onNoEOF():
            self.assert_(False)
        Player.loadFile("empty.avg")
        Player.getRootNode().appendChild(node)
        node.play()
        node.setEOFCallback(onEOF)
        Player.setTimeout(10000, onNoEOF)
        Player.play()


class VideoTestCase(AVTestCase):
    def __init__(self, testFuncName):
        AVTestCase.__init__(self, testFuncName)
    def testVideoFiles(self):
        def testVideoFile(filename, isThreaded):
            def checkImage(filename):
                if not(isThreaded):
                    self.compareImage("testVideo-"+filename+"1", False)
            Player.loadFile("empty.avg")
            node = Player.createNode("video",
                {"href": "../video/testfiles/"+filename, "threaded": isThreaded})
            Player.getRootNode().appendChild(node)
            self.start(None,
                    (lambda: node.play(),
                     lambda: checkImage(filename),
                     lambda: node.stop()
                    ))
        Player.setFakeFPS(25)
        for filename in ["mjpeg-48x48.avi", "mpeg1-48x48.mpg", "mpeg1-48x48-sound.avi", 
                "rgba-48x48.mov", "h264-48x48.h264"]:
            for isThreaded in [False, True]:
                testVideoFile(filename, isThreaded)

    def testVideo(self):
        def newHRef():
            node = Player.getElementByID("clogo2")
            node.href = "../video/testfiles/h264-48x48.h264"
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
                 lambda: self.compareImage("testVideo9", False)
                ))

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
                 None,
                 lambda: self.compareImage("testVideoSeek2", False),
                 lambda: Player.getElementByID("clogo2").play(),
                 None,
                 lambda: self.compareImage("testVideoSeek3", False)
                ))

    def testVideoFPS(self):
        Player.setFakeFPS(25)
        self.start("videofps.avg",
                (lambda: Player.getElementByID("video").play(),
                 None,
                 lambda: self.compareImage("testVideoFPS", False)
                ))

    def testVideoEOF(self):
        Player.setFakeFPS(25)
        for filename in ["mpeg1-48x48.mpg", "mpeg1-48x48-sound.avi"]:
            node = Player.createNode("video",
                    {"href": "../video/testfiles/"+filename})
            self.testEOF(node)

class SoundTestCase(AVTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)
    def testSound(self):
        def testSoundFile(filename):
            Player.loadFile("empty.avg")
            node = Player.createNode("sound",
                    {"href": "../video/testfiles/"+filename})
            Player.getRootNode().appendChild(node)
            self.start(None,
                    (lambda: node.play(),
                     None,
                     lambda: node.stop(),
                     lambda: node.play(),
                     lambda: node.pause(),
                     lambda: node.play(),
                     lambda: node.pause(),
                     lambda: node.stop(),
                     lambda: node.pause()
                    ))
        Player.setFakeFPS(-1)
        Player.volume = 0 
        for filename in ["22.050Hz_16bit_mono.wav", "44.1kHz_16bit_stereo.aif", 
                "44.1kHz_16bit_stereo.wav", "44.1kHz_mono.ogg", "44.1kHz_stereo.mp3",
                "48kHz_24bit_stereo.wav"]:
            testSoundFile(filename)
    def testSoundEOF(self):
        Player.setFakeFPS(-1)
        Player.volume = 0 
        node = Player.createNode("sound",
                {"href": "../video/testfiles/44.1kHz_16bit_mono.wav"})
        self.testEOF(node)


def avTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(SoundTestCase("testSound"))
    suite.addTest(SoundTestCase("testSoundEOF"))
    suite.addTest(VideoTestCase("testVideoFiles"))
    suite.addTest(VideoTestCase("testVideo"))
    suite.addTest(VideoTestCase("testVideoSeek"))
    suite.addTest(VideoTestCase("testVideoFPS"))
    suite.addTest(VideoTestCase("testVideoEOF"))
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
    Player = avg.Player()
    runner = unittest.TextTestRunner()
    rc = runner.run(avTestSuite())
#    while rc.wasSuccessful:
#        rc = runner.run(avTestSuite())
    if rc.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)
        
