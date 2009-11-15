#!/usr/bin/python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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

import unittest

import sys, os, platform

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

    def setUp(self):
        AVGTestCase.setUp(self)
        Player.enableAudio(True)

    def testEOF(self, node):
        def onEOF():
            Player.stop()

        def onNoEOF():
            self.assert_(False)

        self._loadEmpty()
        Player.getRootNode().appendChild(node)
        node.play()
        node.setEOFCallback(onEOF)
        Player.setTimeout(10000, onNoEOF)
        Player.play()

    def testVideoInfo(self):
        def checkInfo():
            node.pause()
            self.assert_(node.getDuration() == 1000)
            self.assert_(node.getBitrate() == 224064)
            self.assert_(node.getVideoCodec() == "mpeg4")
            self.assert_(node.getStreamPixelFormat() == "yuv420p")
            if isThreaded:
                self.assert_(node.getAudioCodec() == "mp2")
                self.assert_(node.getAudioSampleRate() == 44100)
                self.assert_(node.getNumAudioChannels() == 2)

        def checkExceptions():
            node = Player.createNode("video",
                {"href": "../video/testfiles/mpeg1-48x48.mpg",
                        "threaded": isThreaded})
            self.assertException(node.getDuration)
            self.assertException(node.getBitrate)
            self.assertException(node.getVideoCodec)
            self.assertException(node.getStreamPixelFormat)
            node.pause()
            self.assertException(node.getAudioCodec)
            self.assertException(node.getAudioSampleRate)
            self.assertException(node.getNumAudioChannels)
            Player.getRootNode().appendChild(node)

        def checkAudioFile():
            node = Player.createNode("video",
                {"href": "../video/testfiles/44.1kHz_16bit_stereo.wav",
                        "threaded": isThreaded})
            Player.getRootNode().appendChild(node)
            self.assertException(node.pause)

        for isThreaded in (False, True):
            self._loadEmpty()
            node = Player.createNode("video",
                {"href": "../video/testfiles/mpeg1-48x48-sound.avi",
                        "threaded": isThreaded})
            Player.getRootNode().appendChild(node)
            checkInfo()
            checkExceptions()
            self.start(None,
                    (checkInfo,
                     checkExceptions,
                     checkAudioFile,
                    ))

    def testVideoFiles(self):
        def testVideoFile(filename, isThreaded):
            def setVolume(volume):
                node.volume = volume

            def testGetVolume(volume):
                self.assert_(node.volume == volume)

            def checkImage(filename):
                if not(isThreaded):
                    self.compareImage("testVideo-"+filename+"1", False)

            def testHasAudio():
                if filename == "mpeg1-48x48-sound.avi" and isThreaded:
                    self.assert_(node.hasAudio())
                else:
                    self.assert_(not(node.hasAudio()))

            self._loadEmpty()
            node = Player.createNode("video",
                {"href": "../video/testfiles/"+filename, "volume":0.8,
                        "threaded": isThreaded})
            setVolume(0.6)
            Player.getRootNode().appendChild(node)
            self.assertException(node.hasAudio)
            self.start(None,
                    (lambda: setVolume(0.5),
                     lambda: testGetVolume(0.5),
                     lambda: node.play(),
                     lambda: checkImage(filename),
                     lambda: setVolume(0.3),
                     lambda: testGetVolume(0.3),
                     testHasAudio,
                     lambda: node.stop()
                    ))

        for filename in ["mjpeg-48x48.avi", "mpeg1-48x48.mpg", "mpeg1-48x48-sound.avi", 
                "rgba-48x48.mov", "h264-48x48.h264"]:
            for isThreaded in [False, True]:
                testVideoFile(filename, isThreaded)

    def testVideoState(self):
        self._loadEmpty()
        node = Player.createNode("video",
            {"href": "../video/testfiles/mpeg1-48x48.mpg", "threaded": False})
        Player.getRootNode().appendChild(node)
        Player.setFakeFPS(25)
        self.start(None,
                (lambda: node.play(),
                 lambda: self.compareImage("testVideoState1", False),
                 lambda: node.pause(),
                 lambda: self.compareImage("testVideoState2", False),
                 lambda: self.compareImage("testVideoState2", False),
                 lambda: node.play(),
                 lambda: self.compareImage("testVideoState3", False),
                 lambda: node.stop(),
                 lambda: self.compareImage("testVideoState4", False),
                 lambda: node.pause(),
                 lambda: self.compareImage("testVideoState5", False),
                 lambda: self.compareImage("testVideoState5", False),
                 lambda: node.stop(),
                 lambda: self.compareImage("testVideoState4", False),
                ))

    def testVideoActive(self):
        def deactivate():
            node.active=0

        def activate():
            node.active=1
        
        self._loadEmpty()
        node = Player.createNode("video",
            {"href": "../video/testfiles/mpeg1-48x48.mpg", "threaded": False})
        Player.getRootNode().appendChild(node)
        Player.setFakeFPS(25)
        self.start(None,
                (lambda: node.play(),
                 deactivate,
                 lambda: self.compareImage("testVideoActive1", False),
                 activate,
                 lambda: self.compareImage("testVideoActive2", False)
                ))
       
    def testVideoHRef(self):
        def testGetMediaSize():
            self.assert_(node.getMediaSize() == (48, 48))

        def setHRefLoaded():
            node.href = "../video/testfiles/h264-48x48.h264"

        def setHRefUnloaded():
            node = Player.createNode("video", {})
            node.href = "../video/testfiles/h264-48x48.h264"
            node.play()

        def testVideoNotFound():
            # Missing file, but no play() or pause(): Should just work.
            node = Player.createNode("video", {"href":"MissingFile.mov"})
            node.href = "SecondMissingFile.mov"
            # Now libavg notices the missing file.
            self.assertException(node.play)

        self._loadEmpty()
        node = Player.createNode("video",
            {"href": "../video/testfiles/mpeg1-48x48.mpg", "threaded": False})
        Player.getRootNode().appendChild(node)
        Player.setFakeFPS(25)
        testVideoNotFound()
        setHRefUnloaded()
        self.start(None,
                (lambda: node.play(),
                 testGetMediaSize,
                 setHRefLoaded,
                 lambda: self.compareImage("testVideoHRef1", False),
                 testGetMediaSize,
                 testVideoNotFound
                ))

    def testVideoOpacity(self):
        def testWithFile(filename, testImgName):
            def hide():
                Player.getElementByID("video").opacity=0

            def show():
                Player.getElementByID("video").opacity=1

            Player.setFakeFPS(25)
            Player.loadString("""
                <avg width="160" height="120">
                    <video id="video" x="0" y="0" loop="true" threaded="false"/>
                </avg>""")
            Player.getElementByID("video").href=filename
            self.start(None,
                    (lambda: Player.getElementByID("video").play(),
                     lambda: self.compareImage(testImgName+"1", False),
                     hide,
                     None,
                     None,
                     show,
                     lambda: self.compareImage(testImgName+"2", False)
                    ))
        testWithFile("../video/testfiles/rgba-48x48.mov", "testVideoOpacityRGBA")
        testWithFile("../video/testfiles/mpeg1-48x48.mpg", "testVideoOpacityYUV")

    def testVideoSeek(self):
        def seek(frame):
            videoNode.seekToFrame(frame)

        for useCustomFPS in [False, True]:
            Player.setFakeFPS(25)
            self._loadEmpty()
            if useCustomFPS:
                videoNode = Player.createNode("video", 
                        {"loop":True, "fps":25, "threaded":False, 
                         "href":"../video/testfiles/mjpeg-48x48.avi"})
            else:
                videoNode = Player.createNode("video",
                        {"loop":True, "href":"../video/testfiles/mjpeg-48x48.avi",
                         "threaded":False})
            Player.getRootNode().appendChild(videoNode)
            self.start(None,
                    (lambda: videoNode.play(),
                     lambda: seek(100),
                     lambda: self.compareImage("testVideoSeek1", False),
                     lambda: videoNode.pause(),
                     lambda: seek(26),
                     None,
                     lambda: self.compareImage("testVideoSeek2", False),
                     lambda: videoNode.play(),
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

    def testVideoMask(self):
        def testWithFile(filename, testImgName):
            def setMask(href):
                try:
                    video.maskhref = href
                except RuntimeError:
                    print "Skipping testVideoMask - no shader support."
                    Player.stop()
                    skipping = True

            def setOpacity():
                video.opacity = 0.5

            Player.setFakeFPS(25)
            Player.loadString("""
                <avg width="160" height="120">
                    <video id="video" x="0" y="0" opacity="1" threaded="false"/>
                </avg>""")
            video = Player.getElementByID("video")
            video.href = filename
            video.play()
            self.start(None,
                    [lambda: setMask("mask.png"),
                     lambda: self.compareImage(testImgName+"1", False),
                     lambda: video.seekToFrame(10),
                     lambda: setMask(""),
                     lambda: self.compareImage(testImgName+"2", False),
                     lambda: setMask("mask2.png"),
                     lambda: self.compareImage(testImgName+"3", False),
                     setOpacity,
                     lambda: self.compareImage(testImgName+"4", False),
                    ])

        skipping = False
        testWithFile("../video/testfiles/mpeg1-48x48.mpg", "testVideoMaskYUV")
        if not skipping:
            testWithFile("../video/testfiles/mjpeg-48x48.avi", "testVideoMaskYUVJ")
            testWithFile("../video/testfiles/rgba-48x48.mov", "testVideoMaskRGBA")

    def testVideoEOF(self):
        Player.setFakeFPS(25)
        for filename in ["mpeg1-48x48.mpg", "mpeg1-48x48-sound.avi"]:
            node = Player.createNode("video",
                    {"href": "../video/testfiles/"+filename})
            self.testEOF(node)

        Player.loadString("""
            <avg width="160" height="120">
                <video id="video" x="0" y="0" opacity="1" threaded="false"
                        href="../video/testfiles/mpeg1-48x48.mpg"/>
            </avg>""")
        video = Player.getElementByID("video")
        Player.setFakeFPS(0.1)

        video.setEOFCallback(lambda: foo) # Should never be called
        self.start(None,
                [lambda: video.setEOFCallback(None), 
                 video.play,
                 None
                ])


    def testSound(self):
        def testSoundFile(filename):
            def setVolume(volume):
                node.volume = volume

            def testGetVolume(volume):
                self.assert_(node.volume == volume)

            self._loadEmpty()
            node = Player.createNode("sound",
                    {"href": "../video/testfiles/"+filename})
            Player.getRootNode().appendChild(node)
            self.start(None,
                    (
                     lambda: setVolume(0.5),
                     lambda: testGetVolume(0.5),
                     lambda: node.play(),
                     None,
                     lambda: node.stop(),
                     lambda: node.play(),
                     lambda: node.pause(),
                     lambda: node.play(),
                     lambda: setVolume(0.5),
                     lambda: testGetVolume(0.5),
                     lambda: node.pause(),
                     lambda: node.stop(),
                     lambda: setVolume(0.3),
                     lambda: testGetVolume(0.3),
                     lambda: node.pause()
                    ))
        Player.setFakeFPS(-1)
        Player.volume = 0 
        for filename in ["22.050Hz_16bit_mono.wav", "44.1kHz_16bit_stereo.aif", 
                "44.1kHz_16bit_stereo.wav", "44.1kHz_mono.ogg", "44.1kHz_stereo.mp3", 
                "48kHz_24bit_stereo.wav"]:
            testSoundFile(filename)

    def testSoundInfo(self):
        def checkInfo():
            node.pause()
            self.assert_(node.getAudioCodec() == "pcm_s16le")
            self.assert_(node.getAudioSampleRate() == 44100)
            self.assert_(node.getNumAudioChannels() == 2)

        def checkExceptions():
            node = Player.createNode("sound",
                {"href": "../video/testfiles/44.1kHz_16bit_stereo.wav"})
            self.assertException(node.getAudioCodec)
            self.assertException(node.getAudioSampleRate)
            self.assertException(node.getNumAudioChannels)

        def checkVideoFile():
            node = Player.createNode("sound",
                {"href": "../video/testfiles/mpeg1-48x48.mpg"})
            Player.getRootNode().appendChild(node)
            self.assertException(node.pause)

        self._loadEmpty()
        node = Player.createNode("sound",
            {"href": "../video/testfiles/44.1kHz_16bit_stereo.wav"})
        Player.getRootNode().appendChild(node)
        checkInfo()
        checkExceptions()
        self.start(None,
                (checkInfo,
                 checkExceptions,
                 checkVideoFile,
                ))

    def testBrokenSound(self):
        def openSound():
            node = Player.createNode("sound",
                    {"href": "../video/testfiles/44.1kHz_16bit_6Chan.ogg"})
            Player.getRootNode().appendChild(node)
            self.assertException(node.play)

        self._loadEmpty()
        self.start(None,
                [openSound])

    def testSoundEOF(self):
        Player.setFakeFPS(-1)
        Player.volume = 0 
        node = Player.createNode("sound",
                {"href": "../video/testfiles/44.1kHz_16bit_mono.wav"})
        self.testEOF(node)


def AVTestSuite(tests):
    availableTests = (
            "testSound",
            "testSoundInfo",
            "testBrokenSound",
            "testSoundEOF",
            "testVideoInfo",
            "testVideoFiles",
            "testVideoState",
            "testVideoActive",
            "testVideoHRef",
            "testVideoOpacity",
            "testVideoSeek",
            "testVideoFPS",
            "testVideoMask",
            "testVideoEOF",
            )
    return AVGTestSuite(availableTests, AVTestCase, tests)

Player = avg.Player.get()

if __name__ == '__main__':
    runStandaloneTest(AVTestSuite)

