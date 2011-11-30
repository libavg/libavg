#!/usr/bin/python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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

from libavg import avg
from testcase import *

class AVTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def setUp(self):
        AVGTestCase.setUp(self)

    def testEOF(self, node):
        def onEOF():
            Player.stop()

        def onNoEOF():
            self.fail("No EOF")

        root = self.loadEmptyScene()
        root.appendChild(node)
        node.play()
        node.setEOFCallback(onEOF)
        Player.setTimeout(100000, onNoEOF)
        Player.play()
        
    def testVideoInfo(self):
        def checkInfo():
            node.pause()
            self.assertEqual(node.getCurFrame(), 0)
            self.assertEqual(node.getCurTime(), 0)
            self.assertEqual(node.getDuration(), 1000)
            self.assertEqual(node.getBitrate(), 224064)
            self.assertEqual(node.getVideoCodec(), "mpeg4")
            self.assertEqual(node.getStreamPixelFormat(), "yuv420p")
            if isThreaded:
                self.assertEqual(node.getAudioCodec(), "mp2")
                self.assertEqual(node.getAudioSampleRate(), 44100)
                self.assertEqual(node.getNumAudioChannels(), 2)

        def checkExceptions():
            node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg",
                    threaded=isThreaded)
            self.assertException(node.getDuration)
            self.assertException(node.getBitrate)
            self.assertException(node.getVideoCodec)
            self.assertException(node.getStreamPixelFormat)
            node.pause()
            self.assertException(node.getAudioCodec)
            self.assertException(node.getAudioSampleRate)
            self.assertException(node.getNumAudioChannels)
            root.appendChild(node)

        def checkAudioFile():
            node = avg.VideoNode(href="../video/testfiles/44.1kHz_16bit_stereo.wav",
                    threaded=isThreaded, parent=root)
            self.assertException(node.pause)

        for isThreaded in (False, True):
            root = self.loadEmptyScene()
            node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48-sound.avi",
                        threaded=isThreaded, parent=root)
            checkInfo()
            checkExceptions()
            self.start((
                     checkInfo,
                     checkExceptions,
                     checkAudioFile,
                    ))
        root = self.loadEmptyScene()
        node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48-sound.avi",
                    queuelength=23, parent=root)
        self.assertEqual(node.queuelength, 23)

    def testVideoFiles(self):
        def testVideoFile(filename, isThreaded):
            def setVolume(volume):
                node.volume = volume

            def testGetVolume(volume):
                self.assertAlmostEqual(node.volume, volume)

            def checkImage(filename):
                if not(isThreaded):
                    self.compareImage("testVideo-"+filename+"1", False)

            def testInfo():
                if filename == "mpeg1-48x48-sound.avi" and isThreaded:
                    self.assert_(node.hasAudio())
                else:
                    self.assert_(not(node.hasAudio()))
                self.assert_((filename == "rgba-48x48.mov" or 
                        filename == "vp6a-yuva-48x48.flv") == node.hasAlpha())

            root = self.loadEmptyScene()
            node = avg.VideoNode(href="../video/testfiles/"+filename, volume=0.8,
                        size=(96,96), threaded=isThreaded)
            self.assertEqual(node.threaded, isThreaded)
            setVolume(0.6)
            root.appendChild(node)
            self.assertException(node.hasAudio)
            self.start((
                     lambda: setVolume(0.5),
                     lambda: testGetVolume(0.5),
                     lambda: node.play(),
                     lambda: checkImage(filename),
                     lambda: setVolume(0.3),
                     lambda: testGetVolume(0.3),
                     testInfo,
                     lambda: node.stop()
                    ))
        videoFiles = ["mjpeg-48x48.avi", "mpeg1-48x48.mpg", "mpeg1-48x48-sound.avi", 
                "rgba-48x48.mov", "h264-48x48.h264", "vp6a-yuva-48x48.flv"]
        print
        for filename in videoFiles:
            print "  " + filename
            for isThreaded in [False, True]:
                print "    threaded: ", isThreaded
                testVideoFile(filename, isThreaded)

    def testPlayBeforeConnect(self):
        node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", threaded=False)
        node.play()
        root = self.loadEmptyScene()
        root.insertChild(node, 0)
        Player.setFakeFPS(25)
        self.start((
                 lambda: self.assertEqual(node.size, (48, 48)),
                 lambda: self.compareImage("testPlayBeforeConnect", False),
                )) 

    def testVideoState(self):
        for accelerated in [True, False]:
            root = self.loadEmptyScene()
            node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", size=(96,96), 
                    threaded=False, accelerated=accelerated, parent=root)
            Player.setFakeFPS(25)
            self.start((
                     lambda: node.play(),
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
        
        root = self.loadEmptyScene()
        node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", size=(96,96),
                threaded=False, parent=root)
        Player.setFakeFPS(25)
        self.start((
                 lambda: node.play(),
                 deactivate,
                 lambda: self.compareImage("testVideoActive1", False),
                 activate,
                 lambda: self.compareImage("testVideoActive2", False)
                ))

    def testVideoHRef(self):
        def testGetMediaSize():
            self.assertEqual(node.getMediaSize(), (48, 48))

        def setHRefLoaded():
            node.href = "../video/testfiles/h264-48x48.h264"

        def setHRefUnloaded():
            node = avg.VideoNode()
            node.href = "../video/testfiles/h264-48x48.h264"
            node.play()

        def testVideoNotFound():
            # Missing file, but no play() or pause(): Should just work.
            node = avg.VideoNode(href="MissingFile.mov")
            node.href = "SecondMissingFile.mov"
            # Now libavg notices the missing file.
            self.assertException(node.play)

        root = self.loadEmptyScene()
        node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", threaded=False,
                parent=root)
        Player.setFakeFPS(25)
        testVideoNotFound()
        setHRefUnloaded()
        self.start((
                 lambda: node.play(),
                 testGetMediaSize,
                 setHRefLoaded,
                 lambda: self.compareImage("testVideoHRef1", False),
                 testGetMediaSize,
                 testVideoNotFound
                ))

    def testVideoOpacity(self):
        def testWithFile(filename, testImgName):
            def hide():
                self.videoNode.opacity=0

            def show():
                self.videoNode.opacity=1

            Player.setFakeFPS(25)
            root = self.loadEmptyScene()
            self.videoNode = avg.VideoNode(href=filename, loop=True, threaded=False,
                    parent=root)
            self.start((
                     lambda: self.videoNode.play(),
                     None,
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

        def checkCurFrame():
            self.assertEqual(videoNode.getCurFrame(), 26)

        Player.setFakeFPS(25)
        for useCustomFPS in [False, True]:
            root = self.loadEmptyScene()
            if useCustomFPS:
                videoNode = avg.VideoNode(parent=root, loop=True, size=(96,96), fps=25,
                        threaded=False, href="../video/testfiles/mjpeg-48x48.avi")
            else:
                videoNode = avg.VideoNode(parent=root, loop=True, size=(96,96), 
                        threaded=False, href="../video/testfiles/mjpeg-48x48.avi")

            videoNode.play()
            seek(26)
            self.start((
                     checkCurFrame,
                     lambda: self.compareImage("testVideoSeek0", False),
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

        def checkSeek():
            seek(26)
            self.assertNotEqual(videoNode.getCurFrame(), 0)

        root = self.loadEmptyScene()
        videoNode = avg.VideoNode(parent=root, loop=True, fps=25,
                href="../video/testfiles/mjpeg-48x48.avi")
        videoNode.play()
        seek(5)
        self.start((checkSeek,))

    def testVideoFPS(self):
        Player.setFakeFPS(25)
        root = self.loadEmptyScene()
        root = root
        videoNode = avg.VideoNode(size=(80,80), loop=True, threaded=False,
                href="../video/testfiles/mjpeg-48x48.avi", fps=250, parent=root)
        self.start((
                 lambda: videoNode.play(),
                 None,
                 lambda: self.compareImage("testVideoFPS", False)
                ))

    def testVideoLoop(self):
        def onEOF():
            self.eof = True

        def onFrame():
            if self.eof:
                if not(threaded):
                    self.compareImage("testVideoLoop", False)
                Player.stop()

        for threaded in [False, True]:
            self.eof = False
            Player.setFakeFPS(25)
            root = self.loadEmptyScene()
            videoNode = avg.VideoNode(parent=root, loop=True, fps=25, size=(96,96),
                    threaded=threaded, href="../video/testfiles/mpeg1-48x48.mpg")
            videoNode.setEOFCallback(onEOF)
            videoNode.play()
            Player.setOnFrameHandler(onFrame)
            Player.play()

    def testVideoMask(self):
        def testWithFile(filename, testImgName):
            def setMask(href):
                video.maskhref = href

            def setOpacity():
                video.opacity = 0.5

            Player.setFakeFPS(25)
            root = self.loadEmptyScene()
            video = avg.VideoNode(href=filename, threaded=False,
                    parent=root)
            video.play()
            self.start([
                     lambda: setMask("mask.png"),
                     lambda: self.compareImage(testImgName+"1", False),
                     lambda: video.seekToFrame(10),
                     lambda: setMask(""),
                     lambda: self.compareImage(testImgName+"2", False),
                     lambda: setMask("mask2.png"),
                     lambda: self.compareImage(testImgName+"3", False),
                     setOpacity,
                     lambda: self.compareImage(testImgName+"4", False),
                    ])

        if not(self._hasShaderSupport()):
            return
        
        testWithFile("../video/testfiles/mpeg1-48x48.mpg", "testVideoMaskYUV")
        testWithFile("../video/testfiles/mjpeg-48x48.avi", "testVideoMaskYUVJ")
        testWithFile("../video/testfiles/rgba-48x48.mov", "testVideoMaskRGBA")

    def testException(self):
        class TestException(Exception):
            pass
        
        def throwException():
            raise TestException
        
        Player.setFakeFPS(0.1)
        videoNode = avg.VideoNode(threaded = False)
        videoNode.href = "./testmediadir/mjpeg-48x48.avi"
        videoNode.setEOFCallback(throwException)
        
        root = self.loadEmptyScene()
        avg.Player.get().getRootNode().appendChild(videoNode)
        
        self.__exceptionThrown = False
        try:
            self.start((
                 videoNode.pause,
                 lambda: videoNode.seekToFrame(videoNode.getNumFrames()),
                 videoNode.play,
                 lambda: None))
        except TestException:
            self.__exceptionThrown = True
            
        self.assert_(self.__exceptionThrown)
        
    def testVideoEOF(self):
        Player.setFakeFPS(25)
        for filename in ["mpeg1-48x48.mpg", "mpeg1-48x48-sound.avi"]:
            node = avg.VideoNode(href="../video/testfiles/"+filename)
            self.testEOF(node)
        node = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", opacity=0)
        self.testEOF(node)

        root = self.loadEmptyScene()
        video = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", threaded=False,
                parent=root)
        Player.setFakeFPS(0.1)

        video.setEOFCallback(lambda: foo) # Should never be called
        self.start([
                 lambda: video.setEOFCallback(None), 
                 video.play,
                 None
                ])


    def testSound(self):
        def testSoundFile(filename):
            def setVolume(volume):
                node.volume = volume

            def testGetVolume(volume):
                self.assertAlmostEqual(node.volume, volume)

            root = self.loadEmptyScene()
            node = avg.SoundNode(href="../video/testfiles/"+filename, 
                    parent=root)
            self.start((
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
            self.assertEqual(node.getAudioCodec(), "pcm_s16le")
            self.assertEqual(node.getAudioSampleRate(), 44100)
            self.assertEqual(node.getNumAudioChannels(), 2)

        def checkExceptions():
            node = avg.SoundNode(href="../video/testfiles/44.1kHz_16bit_stereo.wav")
            self.assertException(node.getAudioCodec)
            self.assertException(node.getAudioSampleRate)
            self.assertException(node.getNumAudioChannels)

        def checkVideoFile():
            node = avg.SoundNode(href="../video/testfiles/mpeg1-48x48.mpg",
                    parent=root)
            self.assertException(node.pause)

        root = self.loadEmptyScene()
        node = avg.SoundNode(href="../video/testfiles/44.1kHz_16bit_stereo.wav",
                    parent=root)
        checkInfo()
        checkExceptions()
        self.start((
                 checkInfo,
                 checkExceptions,
                 checkVideoFile,
                ))

    def testBrokenSound(self):
        def openSound():
            node = avg.SoundNode(href="../video/testfiles/44.1kHz_16bit_6Chan.ogg",
                    parent=root)
            self.assertException(node.play)

        root = self.loadEmptyScene()
        self.start([openSound])

    def testSoundEOF(self):
        Player.setFakeFPS(-1)
        Player.volume = 0 
        node = avg.SoundNode(href="../video/testfiles/44.1kHz_16bit_mono.wav")
        self.testEOF(node)

    def testVideoWriter(self):
        
        def startWriter(fps, syncToPlayback):
            self.videoWriter = avg.VideoWriter(canvas, "test.mov", fps, 3, 5, 
                    syncToPlayback)

        def stopWriter():
            self.videoWriter.stop()

        def killWriter():
            self.videoWriter = None

        def pauseWriter():
            self.videoWriter.pause()

        def playWriter():
            self.videoWriter.play()

        def hideVideo():
            videoNode.opacity = 0

        def showVideo():
            videoNode.opacity = 1

        def checkVideo(numFrames):
            savedVideoNode = avg.VideoNode(href="test.mov", pos=(48,0), threaded=False, 
                    parent=root)
            savedVideoNode.pause()
            self.assertEqual(savedVideoNode.getVideoCodec(), "mjpeg")
            self.assertEqual(savedVideoNode.getNumFrames(), numFrames)
            self.assertEqual(savedVideoNode.getStreamPixelFormat(), "yuvj420p")

        def testCreateException():
            self.assertException(lambda: avg.VideoWriter(Player.getMainCanvas(), 
                    "nonexistentdir/test.mov", 30))

        if not(self._isCurrentDirWriteable()):
            self.skip("Current dir not writeable")
            return

        for useCanvas in (False, True):
            Player.setFakeFPS(30)
            
            root = self.loadEmptyScene()
            videoNode = avg.VideoNode(href="../video/testfiles/mpeg1-48x48.mpg", 
                    threaded=False)
            if useCanvas:
                canvas = Player.createCanvas(id="canvas", size=(48,48))
                canvas.getRootNode().appendChild(videoNode)
                avg.ImageNode(parent=root, href="canvas:canvas")
                testImageName = "testVideoWriterCanvas"
            else:
                root.appendChild(videoNode)
                canvas = Player.getMainCanvas()
                testImageName = "testVideoWriter"

            self.start((
                 videoNode.play,
                 lambda: startWriter(30, True),
                 lambda: self.delay(66),
                 stopWriter,
                 killWriter,
                 lambda: checkVideo(4),
                 hideVideo,
                 lambda: self.compareImage(testImageName+"1", False),
                 showVideo,
                 testCreateException,
                 lambda: startWriter(15, False),
                 lambda: self.delay(100),
                 stopWriter,
                 killWriter,
                 lambda: checkVideo(2),
                 lambda: startWriter(30, False),
                 pauseWriter,
                 lambda: self.delay(200),
                 playWriter,
                 stopWriter,
                 killWriter,
                 lambda: checkVideo(1),
                 lambda: startWriter(30, False),
                 killWriter,
                 lambda: checkVideo(1),
                ))
            os.remove("test.mov")    

    def test2VideosAtOnce(self):
        Player.setFakeFPS(25)
        self.loadEmptyScene()
        root = Player.getRootNode()
        for pos in ((0,0), (80,0)):
            video = avg.VideoNode(pos=pos, threaded=False, 
                    href="../video/testfiles/mpeg1-48x48.mpg", parent=root)
            video.play()
        self.start([lambda: self.compareImage("test2VideosAtOnce1", False),])

    def testVideoAccel(self):
        accelConfig = avg.VideoNode.getVideoAccelConfig()
        video = avg.VideoNode(threaded=False, accelerated=False, 
                href="../video/testfiles/mpeg1-48x48.mpg")
        video.play()
        self.assertEqual(video.accelerated, False)
        video = avg.VideoNode(threaded=False, accelerated=True, 
                href="../video/testfiles/mpeg1-48x48.mpg")
        video.play()
        self.assertEqual(video.accelerated, (accelConfig != avg.NO_ACCELERATION))


def AVTestSuite(tests):
    availableTests = [
            "testSound",
            "testSoundInfo",
            "testBrokenSound",
            "testSoundEOF",
            "testVideoInfo",
            "testVideoFiles",
            "testPlayBeforeConnect",
            "testVideoState",
            "testVideoActive",
            "testVideoHRef",
            "testVideoOpacity",
            "testVideoSeek",
            "testVideoFPS",
            "testVideoLoop",
            "testVideoMask",
            "testVideoEOF",
            "testException",
            "testVideoWriter",
            "test2VideosAtOnce",
            "testVideoAccel",
            ]
    return createAVGTestSuite(availableTests, AVTestCase, tests)

Player = avg.Player.get()
