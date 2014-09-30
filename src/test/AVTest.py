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

class AVTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def setUp(self):
        AVGTestCase.setUp(self)

    def testEOF(self, node):
        def onEOF():
            player.stop()

        def onNoEOF():
            self.fail("No EOF")

        def onSubscribeEOF():
            self.eofCalled = True

        self.eofCalled = False
        root = self.loadEmptyScene()
        root.appendChild(node)
        node.play()
        node.setEOFCallback(onEOF)
        node.subscribe(avg.Node.END_OF_FILE, onSubscribeEOF)
        player.setTimeout(100000, onNoEOF)
        player.play()
        self.assert_(self.eofCalled)

    def testVideoInfo(self):
        def checkInfo():
            node.pause()
            self.assertEqual(node.getContainerFormat(), "avi")
            self.assertEqual(node.getCurFrame(), 0)
            self.assertEqual(node.getCurTime(), 0)
            self.assertEqual(node.getDuration(), 1000)
            self.assertEqual(node.getBitrate(), 224064)
            self.assertEqual(node.getVideoCodec(), "mpeg4")
            self.assertEqual(node.getStreamPixelFormat(), "yuv420p")
            self.assertEqual(node.getVideoDuration(), 1000)
            if isThreaded:
                self.assertEqual(node.getAudioCodec(), "mp2")
                self.assertEqual(node.getAudioSampleRate(), 44100)
                self.assertEqual(node.getNumAudioChannels(), 2)
                self.assert_(node.getVideoDuration() >= 1000)

        def checkEnableSound():
            node = avg.VideoNode(href="mpeg1-48x48-sound.avi", threaded=isThreaded,
                    enablesound=False, parent=root)
            node.pause()
            self.assertEqual(node.getVideoCodec(), "mpeg4")
            self.assertRaises(RuntimeError, node.getAudioCodec)

        def checkExceptions():
            node = avg.VideoNode(href="mpeg1-48x48.mov", threaded=isThreaded)
            self.assertRaises(RuntimeError, node.getDuration)
            self.assertRaises(RuntimeError, node.getBitrate)
            self.assertRaises(RuntimeError, node.getVideoCodec)
            self.assertRaises(RuntimeError, node.getStreamPixelFormat)
            node.pause()
            self.assertRaises(RuntimeError, node.getAudioCodec)
            self.assertRaises(RuntimeError, node.getAudioSampleRate)
            self.assertRaises(RuntimeError, node.getNumAudioChannels)
            root.appendChild(node)

        def checkAudioFile():
            node = avg.VideoNode(href="44.1kHz_16bit_stereo.wav", threaded=isThreaded,
                    parent=root)
            self.assertRaises(RuntimeError, node.pause)

        sys.stderr.write("\n")
        for isThreaded in (False, True):
            sys.stderr.write("  Threaded: " + str(isThreaded) + "\n")
            root = self.loadEmptyScene()
            node = avg.VideoNode(href="mpeg1-48x48-sound.avi", threaded=isThreaded,
                    parent=root)
            checkInfo()
            checkEnableSound()
            checkExceptions()
            self.start(False,
                    (checkInfo,
                     checkExceptions,
                     checkAudioFile,
                    ))
        sys.stderr.write("  Nonstandard queue length\n")
        root = self.loadEmptyScene()
        node = avg.VideoNode(href="mpeg1-48x48-sound.avi", queuelength=23, parent=root)
        self.assertEqual(node.queuelength, 23)

    def testVideoFiles(self):
        def testVideoFile(filename, isThreaded):
            def setVolume(volume):
                node.volume = volume

            def testGetVolume(volume):
                self.assertAlmostEqual(node.volume, volume)

            def checkImage(filename):
                if not(isThreaded):
                    self.compareImage("testVideo-"+filename+"1")

            def testInfo():
                if filename == "mpeg1-48x48-sound.avi" and isThreaded:
                    self.assert_(node.hasAudio())
                else:
                    self.assert_(not(node.hasAudio()))
                self.assert_((filename == "rgba-48x48.mov" or 
                        filename == "vp6a-yuva-48x48.flv") == node.hasAlpha())

            root = self.loadEmptyScene()
            node = avg.VideoNode(href=filename, volume=0.8, size=(96,96), 
                    threaded=isThreaded)
            self.assertEqual(node.threaded, isThreaded)
            setVolume(0.6)
            root.appendChild(node)
            self.assertRaises(RuntimeError, node.hasAudio)
            self.start(False,
                    (lambda: setVolume(0.5),
                     lambda: testGetVolume(0.5),
                     lambda: node.play(),
                     lambda: checkImage(filename),
                     lambda: setVolume(0.3),
                     lambda: testGetVolume(0.3),
                     testInfo,
                     lambda: node.stop()
                    ))
        videoFiles = ["mjpeg-48x48.avi", "mpeg1-48x48.mov", #"mpeg1-48x48-sound.avi", 
                "rgba-48x48.mov", "h264-48x48.h264", "vp6a-yuva-48x48.flv"]
        sys.stderr.write("\n")
        for filename in videoFiles:
            sys.stderr.write("  "+filename+"\n")
            for isThreaded in [False, True]:
                sys.stderr.write("    threaded: "+str(isThreaded)+"\n")
                testVideoFile(filename, isThreaded)

    def testPlayBeforeConnect(self):
        node = avg.VideoNode(href="media/mpeg1-48x48.mov", threaded=False)
        node.play()
        player.createMainCanvas(size=(160,120))
        root = player.getRootNode()
        root.insertChild(node, 0)
        player.setFakeFPS(25)
        self.start(False,
                (lambda: self.assertEqual(node.size, (48, 48)),
                 lambda: self.compareImage("testPlayBeforeConnect"),
                )) 

    def testVideoState(self):
        for accelerated in [True, False]:
            root = self.loadEmptyScene()
            node = avg.VideoNode(href="mpeg1-48x48.mov", size=(96,96), threaded=False,
                    accelerated=accelerated, parent=root)
            player.setFakeFPS(25)
            self.start(False,
                    (lambda: node.play(),
                     lambda: self.compareImage("testVideoState1"),
                     lambda: node.pause(),
                     lambda: self.compareImage("testVideoState2"),
                     lambda: self.compareImage("testVideoState2"),
                     lambda: node.play(),
                     lambda: self.compareImage("testVideoState3"),
                     lambda: node.stop(),
                     lambda: self.compareImage("testVideoState4"),
                     lambda: node.pause(),
                     lambda: self.compareImage("testVideoState5"),
                     lambda: self.compareImage("testVideoState5"),
                     lambda: node.stop(),
                     lambda: self.compareImage("testVideoState4"),
                    ))

    def testVideoActive(self):
        def deactivate():
            node.active=0

        def activate():
            node.active=1
        
        root = self.loadEmptyScene()
        node = avg.VideoNode(href="mpeg1-48x48.mov", size=(96,96), threaded=False,
                parent=root)
        player.setFakeFPS(25)
        self.start(False,
                (lambda: node.play(),
                 deactivate,
                 lambda: self.compareImage("testVideoActive1"),
                 activate,
                 lambda: self.compareImage("testVideoActive2")
                ))

    def testVideoHRef(self):
        def testGetMediaSize():
            self.assertEqual(node.getMediaSize(), (48, 48))

        def setHRefLoaded():
            node.href = "h264-48x48.h264"

        def setHRefUnloaded():
            node = avg.VideoNode()
            node.href = "h264-48x48.h264"
            node.play()

        def testVideoNotFound():
            # Missing file, but no play() or pause(): Should just work.
            node = avg.VideoNode(href="MissingFile.mov")
            node.href = "SecondMissingFile.mov"
            # Now libavg notices the missing file.
            self.assertRaises(RuntimeError, node.play)

        def testVideoBroken():
            node = avg.VideoNode(href="rgb24-64x64.png")
            self.assertRaises(RuntimeError, node.play)

        root = self.loadEmptyScene()
        node = avg.VideoNode(href="mpeg1-48x48.mov", threaded=False, parent=root)
        player.setFakeFPS(25)
        testVideoNotFound()
        testVideoBroken()
        setHRefUnloaded()
        self.start(False,
                (lambda: node.play(),
                 testGetMediaSize,
                 setHRefLoaded,
                 lambda: self.compareImage("testVideoHRef1"),
                 testGetMediaSize,
                 testVideoNotFound,
                 testVideoBroken
                ))

    def testVideoOpacity(self):
        def testWithFile(filename, testImgName):
            def hide():
                self.videoNode.opacity=0

            def show():
                self.videoNode.opacity=1

            player.setFakeFPS(25)
            root = self.loadEmptyScene()
            self.videoNode = avg.VideoNode(href=filename, loop=True, threaded=False,
                    parent=root)
            self.start(False,
                    (lambda: self.videoNode.play(),
                     None,
                     lambda: self.compareImage(testImgName+"1"),
                     hide,
                     None,
                     None,
                     show,
                     lambda: self.compareImage(testImgName+"2")
                    ))
        testWithFile("rgba-48x48.mov", "testVideoOpacityRGBA")
        testWithFile("mpeg1-48x48.mov", "testVideoOpacityYUV")

    def testVideoSeek(self):
        def seek(frame):
            videoNode.seekToFrame(frame)

        def checkCurFrame():
            self.assertEqual(videoNode.getCurFrame(), 26)

        player.setFakeFPS(25)
        for useCustomFPS in [False, True]:
            root = self.loadEmptyScene()
            if useCustomFPS:
                videoNode = avg.VideoNode(parent=root, loop=True, size=(96,96), fps=25,
                        threaded=False, href="mjpeg-48x48.avi")
            else:
                videoNode = avg.VideoNode(parent=root, loop=True, size=(96,96), 
                        threaded=False, href="mjpeg-48x48.avi")

            videoNode.play()
            seek(26)
            self.start(False,
                    (checkCurFrame,
                     lambda: self.compareImage("testVideoSeek0"),
                     lambda: seek(100),
                     lambda: self.compareImage("testVideoSeek1"),
                     lambda: videoNode.pause(),
                     lambda: seek(26),
                     None,
                     lambda: self.compareImage("testVideoSeek2"),
                     lambda: videoNode.play(),
                     None,
                     lambda: self.compareImage("testVideoSeek3")
                    ))

        def checkSeek():
            seek(26)
            self.assertNotEqual(videoNode.getCurFrame(), 0)

    def testVideoFPS(self):
        player.setFakeFPS(25)
        root = self.loadEmptyScene()
        root = root
        videoNode = avg.VideoNode(size=(80,80), loop=True, threaded=False,
                href="mjpeg-48x48.avi", fps=250, parent=root)
        self.start(False,
                (lambda: videoNode.play(),
                 None,
                 lambda: self.compareImage("testVideoFPS")
                ))

    def testVideoLoop(self):
        def onEOF():
            self.eof = True

        def onFrame():
            if self.eof:
                if not(threaded):
                    self.compareImage("testVideoLoop")
                player.stop()

        for threaded in [False, True]:
            self.eof = False
            player.setFakeFPS(25)
            root = self.loadEmptyScene()
            videoNode = avg.VideoNode(parent=root, loop=True, fps=25, size=(96,96),
                    threaded=threaded, href="mpeg1-48x48.mov")
            videoNode.subscribe(avg.Node.END_OF_FILE, onEOF)
            videoNode.play()
            player.subscribe(player.ON_FRAME, onFrame)
            player.play()

    def testVideoMask(self):
        def testWithFile(filename, testImgName):
            def setMask(href):
                video.maskhref = href

            def setOpacity():
                video.opacity = 0.5
            
            print "  ", filename
            player.setFakeFPS(25)
            root = self.loadEmptyScene()
            video = avg.VideoNode(href=filename, threaded=False,
                    parent=root)
            video.play()
            self.start(False,
                    (lambda: setMask("mask.png"),
                     lambda: self.compareImage(testImgName+"1"),
                     lambda: video.seekToFrame(10),
                     lambda: setMask(""),
                     lambda: self.compareImage(testImgName+"2"),
                     lambda: setMask("mask2.png"),
                     lambda: self.compareImage(testImgName+"3"),
                     setOpacity,
                     lambda: self.compareImage(testImgName+"4"),
                    ))
        
        print
        testWithFile("mpeg1-48x48.mov", "testVideoMaskYUV")
        testWithFile("mjpeg-48x48.avi", "testVideoMaskYUVJ")
        testWithFile("rgba-48x48.mov", "testVideoMaskRGBA")

    def testException(self):
        class TestException(Exception):
            pass
        
        def throwException():
            raise TestException
        
        player.setFakeFPS(0.1)
        videoNode = avg.VideoNode(threaded = False)
        videoNode.href = "../testmediadir/mjpeg-48x48.avi"
        videoNode.subscribe(avg.Node.END_OF_FILE, throwException)
        
        root = self.loadEmptyScene()
        root.appendChild(videoNode)
        
        self.__exceptionThrown = False
        try:
            self.start(False,
                    (videoNode.pause,
                     lambda: videoNode.seekToFrame(videoNode.getNumFrames()),
                     videoNode.play,
                     lambda: None
                    ))
        except TestException:
            self.__exceptionThrown = True
            
        self.assert_(self.__exceptionThrown)
        
    def testVideoEOF(self):
        player.setFakeFPS(25)
        for filename in ["mpeg1-48x48.mov", "mpeg1-48x48-sound.avi"]:
            node = avg.VideoNode(href=filename)
            self.testEOF(node)
        node = avg.VideoNode(href="mpeg1-48x48.mov", opacity=0)
        self.testEOF(node)

        root = self.loadEmptyScene()
        video = avg.VideoNode(href="mpeg1-48x48.mov", threaded=False,
                parent=root)
        player.setFakeFPS(0.1)
       
        # Should never be called
        eofID = video.subscribe(avg.Node.END_OF_FILE, lambda: self.assert_(False))   
        self.start(False,
                (lambda: video.unsubscribe(avg.Node.END_OF_FILE, eofID), 
                 video.play,
                 None
                ))

    def testVideoSeekAfterEOF(self):
        def onEOF():
            node.seekToTime(0)
            player.subscribe(avg.Player.ON_FRAME, onFrame)

        def onFrame():
            if node.getCurTime() < 100:
                self.compareImage("testSeekAfterEOF")
                player.stop()

        def onNoEOF():
            self.fail("No EOF")

        player.setFakeFPS(25)
        root = self.loadEmptyScene()
        node = avg.VideoNode(href="mpeg1-48x48.mov", parent=root)
        node.play()
        node.subscribe(avg.VideoNode.END_OF_FILE, onEOF)
        player.setTimeout(100000, onNoEOF)
        player.play()

    def testSound(self):
        def testSoundFile(filename):
            def setVolume(volume):
                node.volume = volume

            def testGetVolume(volume):
                self.assertAlmostEqual(node.volume, volume)

            root = self.loadEmptyScene()
            node = avg.SoundNode(href=filename, parent=root)
            self.start(False,
                    (lambda: setVolume(0.5),
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
        player.setFakeFPS(-1)
        player.volume = 0
        # "44.1kHz_mono.ogg" not tested for now - broken under Windows.
        # Assuming buggy ffmpeg version. 
        for filename in ["22.050Hz_16bit_mono.wav", "44.1kHz_16bit_stereo.aif", 
                "44.1kHz_16bit_stereo.wav", "44.1kHz_stereo.mp3", 
                "48kHz_24bit_stereo.wav"]:
            testSoundFile(filename)

    def testSoundInfo(self):
        def checkInfo():
            node.pause()
            self.assertEqual(node.getAudioCodec(), "pcm_s16le")
            self.assertEqual(node.getAudioSampleRate(), 44100)
            self.assertEqual(node.getNumAudioChannels(), 2)

        def checkExceptions():
            node = avg.SoundNode(href="44.1kHz_16bit_stereo.wav")
            self.assertRaises(RuntimeError, node.getAudioCodec)
            self.assertRaises(RuntimeError, node.getAudioSampleRate)
            self.assertRaises(RuntimeError, node.getNumAudioChannels)

        def checkVideoFile():
            node = avg.SoundNode(href="mpeg1-48x48.mov", parent=root)
            self.assertRaises(RuntimeError, node.pause)

        root = self.loadEmptyScene()
        node = avg.SoundNode(href="44.1kHz_16bit_stereo.wav", parent=root)
        checkInfo()
        checkExceptions()
        self.start(False,
                (checkInfo,
                 checkExceptions,
                 checkVideoFile,
                ))

    def testSoundSeek(self):
        player.setFakeFPS(-1)
        player.volume = 0 
        root = self.loadEmptyScene()
        soundNode = avg.SoundNode(parent=root, href="44.1kHz_16bit_stereo.wav")
        soundNode.play()
        soundNode.seekToTime(500)
        self.start(False,
                (None,
                 lambda: soundNode.seekToTime(200),
                ))


    def testBrokenSound(self):
        def openSound():
            node = avg.SoundNode(href="44.1kHz_16bit_6Chan.ogg", parent=root)
            self.assertRaises(RuntimeError, node.play)

        root = self.loadEmptyScene()
        self.start(False, [openSound])

    def testSoundEOF(self):
        player.setFakeFPS(-1)
        player.volume = 0 
        node = avg.SoundNode(href="44.1kHz_16bit_mono.wav")
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
            savedVideoNode = avg.VideoNode(href="../test.mov", pos=(48,0), 
                    threaded=False, parent=root)
            savedVideoNode.pause()
            self.assertEqual(savedVideoNode.getVideoCodec(), "mjpeg")
            self.assertEqual(savedVideoNode.getNumFrames(), numFrames)
            self.assertEqual(savedVideoNode.getStreamPixelFormat(), "yuvj420p")

        def testCreateException():
            self.assertRaises(RuntimeError,
                    lambda: avg.VideoWriter(player.getMainCanvas(), 
                            "nonexistentdir/test.mov", 30))

        if not(self._isCurrentDirWriteable()):
            self.skip("Current dir not writeable.")
            return
        if player.isUsingGLES():
            self.skip("VideoWriter not supported under GLES.")
            return

        self.assertRaises(RuntimeError, lambda:
                avg.VideoWriter(player.getMainCanvas(), "test.mov", 30, 3, 5, False))

        for useCanvas in (False, True):
            player.setFakeFPS(30)
            
            root = self.loadEmptyScene()
            videoNode = avg.VideoNode(href="mpeg1-48x48.mov", threaded=False)
            if useCanvas:
                canvas = player.createCanvas(id="canvas", size=(48,48),
                        mediadir="media")
                canvas.getRootNode().appendChild(videoNode)
                avg.ImageNode(parent=root, href="canvas:canvas")
                testImageName = "testVideoWriterCanvas"
            else:
                root.appendChild(videoNode)
                canvas = player.getMainCanvas()
                testImageName = "testVideoWriter"

            self.start(False,
                (videoNode.play,
                 lambda: startWriter(30, True),
                 lambda: self.delay(100),
                 stopWriter,
                 killWriter,
                 lambda: checkVideo(4),
                 hideVideo,
                 lambda: self.compareImage(testImageName+"1"),
                 showVideo,
                 testCreateException,
                 lambda: startWriter(15, False),
                 lambda: self.delay(150),
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
        player.setFakeFPS(25)
        self.loadEmptyScene()
        root = player.getRootNode()
        for pos in ((0,0), (80,0)):
            video = avg.VideoNode(pos=pos, threaded=False, href="mpeg1-48x48.mov",
                    parent=root)
            video.play()
        self.start(False,
                [lambda: self.compareImage("test2VideosAtOnce1"),])

    def testVideoAccel(self):
        accelConfig = avg.VideoNode.getVideoAccelConfig()
        video = avg.VideoNode(accelerated=False, href="media/mpeg1-48x48.mov")
        video.play()
        self.assertEqual(video.accelerated, False)
        video = avg.VideoNode(accelerated=True, href="media/mpeg1-48x48.mov")
        video.play()
        self.assertEqual(video.accelerated, (accelConfig != avg.NO_ACCELERATION))


def AVTestSuite(tests):
    availableTests = [
            "testSound",
            "testSoundInfo",
            "testSoundSeek",
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
            "testVideoSeekAfterEOF",
            "testException",
            "testVideoWriter",
            "test2VideosAtOnce",
            "testVideoAccel",
            ]
    return createAVGTestSuite(availableTests, AVTestCase, tests)

