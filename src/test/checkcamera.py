#!/usr/bin/env python
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
# Original author of this file is Robert Parcus <betoparcus@gmail.com>
#

from libavg import *
import camcfgs
import optparse

from testcase import *

def parseCmdLine():
    global g_TestParams

    validCameras = ('Dragonfly', 'FireflyMV', 'Firei', 'DFx31BF03', 'QuickCamProLinux',
            'QuickCamProBGRWin', 'QuickCamPro9Win')
    parser = optparse.OptionParser(usage=
"""%prog cameraname [option]. 
A test to check camera features support by libavg. Supported cameras are """
+ str(validCameras))
#    parser.add_argument(dest='camera', action='store', type=str, 
#            choices=validCameras, help = 'Select which camera model to test.')
    parser.add_option('--test-params', '-p', dest='testParams', action='store_true',
            default=False, 
            help='Execute optional tests for camera params like gain, shutter, etc.')
    (options, args) = parser.parse_args()
    g_TestParams = options.testParams
    if len(args) != 1:
        parser.error("Must be invoked with camera name as argument")
    cameraName = args[0]
    if cameraName == 'Dragonfly':
        return camcfgs.Dragonfly2
    elif cameraName == 'FireflyMV':
        return camcfgs.FireflyMV
    elif cameraName == 'Firei':
        return camcfgs.Firei 
    elif cameraName == 'DFx31BF03':
        return camcfgs.DFx31BF03Cfg
    elif cameraName == 'QuickCamProLinux':
        return camcfgs.QuickCamProLinux
    elif cameraName == 'QuickCamPro9Win':
        return camcfgs.QuickCamPro9Win
    elif cameraName == 'QuickCamProBGRWin':
        return camcfgs.QuickCamProBGRWin
    else:
        parser.error("Unknown camera name '" + cameraName + "'.")

class CameraTestCase(AVGTestCase):
    def __init__(self, cameraCfg, fmt, testFuncName, *testFuncArgs):
        self.cameraCfg = cameraCfg
        self.fmt = fmt
        self.testFuncArgs = testFuncArgs
        AVGTestCase.__init__(self, testFuncName)
        
    def testFormat(self):
        self.__dumpFormat()
        self.loadEmptyScene()
        self.__openCamera()
        self.actions = [None, None]
        avg.player.setOnFrameHandler(self.__onFrame)
        avg.player.play()
        self.assertEqual(self.cam.framenum, 2)
        self.cam = None

    def testIllegalFormat(self):
        self.loadEmptyScene()
        self.assertRaises(RuntimeError, (lambda: self.__openCamera())

    def testCreateDelete(self):
        # Create and delete without ever calling play.
        self.__openCamera()
        self.cam.unlink(True)

    def testParams(self):
        def buildParamActionList(testCfg):
            actions = []
            actions.append(setDefaultParams)
            actions.append(None)
            for val in testCfg.testValues:
                actions.append(lambda paramName=testCfg.name, val=val: 
                        setCamParam(paramName, val))
                actions.append(None)
                actions.append(None)
                actions.append(None)
                actions.append(lambda name=testCfg.name: calcCamImgAverage(name))
            actions.append(lambda testCfg=testCfg: checkCamImageChange(testCfg))
            return actions

        def setDefaultParams():
            for (paramName, val) in cameraCfg.defaultParams.iteritems():
                if paramName == "setWhitebalance":
                    self.cam.setWhitebalance(*val)
                else:
                    setattr(self.cam, paramName, val)
               
        def setCamParam(paramName, val):
            if paramName == 'setwhitebalance':
                self.cam.setWhitebalance(*val)
            else:
                setattr(self.cam, paramName, val)

        def calcCamImgAverage(param):
            bmp = self.cam.getBitmap()
            self.camBmps.append(bmp)
            if isColorParam(param):
                colour = []
                colour.append(bmp.getChannelAvg(0))
                colour.append(bmp.getChannelAvg(1))
                colour.append(bmp.getChannelAvg(2))
                self.averages.append(colour)
            else:    
                self.averages.append(bmp.getAvg())

        def checkCamImageChange(testCfg):
            
            def saveCamImages():
#                print
#                print "Average image brightnesses: ",minAverages, medAverages, maxAverages
                dir = AVGTestCase.getImageResultDir()
                for (i, category) in enumerate(("min", "med", "max")):
                    self.camBmps[i].save(dir+"/cam"+testCfg.name+category+".png")

            minAverages = self.averages[0]
            medAverages = self.averages[1]
            maxAverages = self.averages[2]
            ok = False
            if isColorParam(testCfg.name):
                pass
            else:
                if minAverages+testCfg.minMedDiff > medAverages:
                    saveCamImages()
                    self.fail()
                if medAverages+testCfg.medMaxDiff > maxAverages:
                    saveCamImages()
                    self.fail()
            self.averages = [] 
            self.camBmps = []

        def isColorParam(paramName):
            return paramName in ['saturation', 'setwhitebalance']

        testCfg = self.testFuncArgs[0]
        print >>sys.stderr, testCfg.name, " ",
        self.loadEmptyScene()
        self.__openCamera()
        self.actions = buildParamActionList(testCfg)
        avg.player.setOnFrameHandler(self.__onFrame)
        self.averages = []
        self.camBmps = []
        avg.player.play()
        self.cam = None

    def __openCamera(self):
        self.cam = avg.CameraNode(driver=self.cameraCfg.driver, 
                device=self.cameraCfg.device, unit=self.cameraCfg.unit,
                fw800=self.cameraCfg.fw800, framerate=self.fmt.framerate,
                capturewidth=self.fmt.size[0], captureheight=self.fmt.size[1],
                pixelformat=self.fmt.pixelformat,
                parent=avg.player.getRootNode())
        self.cam.play()
        self.lastCameraFrame = -1
        self.assert_(self.cam.isAvailable())

    def __onFrame(self):
        # We execute one action per camera frame.
#        print self.cam.framenum
        if self.cam.framenum != self.lastCameraFrame:
            self.lastCameraFrame += 1
            if len(self.actions) == self.lastCameraFrame:
                avg.player.stop()
            else:
                action = self.actions[self.lastCameraFrame]
                if action != None:
                    action()

    def __dumpFormat(self):
        print >>sys.stderr, str(self.fmt.size)+", "+str(self.fmt.pixelformat)+ \
                ", "+str(self.fmt.framerate)+" ",

def dumpCameraCfg(cfg):
    print >>sys.stderr, "Camera config: driver="+cfg.driver+", device="+cfg.device+ \
            ", unit="+str(cfg.unit)


cameraCfg = parseCmdLine()
AVGTestCase.cleanResultDir()
suite = unittest.TestSuite()
dumpCameraCfg(cameraCfg)
for fmt in cameraCfg.formats:
    suite.addTest(CameraTestCase(cameraCfg, fmt, "testFormat"))
suite.addTest(CameraTestCase(cameraCfg, cameraCfg.illegalFormat, "testIllegalFormat"))
suite.addTest(CameraTestCase(cameraCfg, cameraCfg.formats[0], "testCreateDelete"))
if g_TestParams:
    for testCfg in cameraCfg.paramTests:
        suite.addTest(CameraTestCase(cameraCfg, cameraCfg.formats[0], "testParams", 
                testCfg))
testRunner = unittest.TextTestRunner(verbosity = 2)
testResult = testRunner.run(suite)

if testResult.wasSuccessful():
    exitCode = 0
else:
    exitCode = 1
sys.exit(exitCode)


