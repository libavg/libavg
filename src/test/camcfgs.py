#!/usr/bin/env python
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

class CameraTestCfg:
    def __init__(self, driver, device, unit, fw800, formats, illegalFormat, paramTests, 
            defaultParams):
        self.driver = driver
        self.device = device
        self.unit = unit
        self.fw800 = fw800

        self.formats = formats
        self.illegalFormat = illegalFormat
        self.paramTests = paramTests
        self.defaultParams = defaultParams

class CameraFormatCfg:
    def __init__(self, size, pixelformat, framerate):
        self.size = size
        self.pixelformat = pixelformat
        self.framerate = framerate

class ParamTestCfg:
    def __init__(self, name, testValues, minMedDiff, medMaxDiff):
        self.name = name
        self.testValues = testValues
        self.minMedDiff = minMedDiff
        self.medMaxDiff = medMaxDiff

DFx31BF03Cfg = CameraTestCfg('firewire', '', -1, False,
    [CameraFormatCfg((1024,768), 'BAYER8', 30),
     CameraFormatCfg((1024,768), 'I8', 30),
     CameraFormatCfg((1024,768), 'YUV422', 15),
     CameraFormatCfg((1024,768), 'YUV422', 7.5)],
    CameraFormatCfg((320,240), 'BAYER8', 30),
    [ParamTestCfg('gain', (180, 800, 1023), 6, 6),
     ParamTestCfg('camgamma', (10, 16, 22), 15, 15),
     ParamTestCfg('brightness', (0, 127, 254), 8, 8)],
    {'gain':300, 'shutter':220, 'saturation':1200, 'camgamma':16,
     'brightness':120, 'setWhitebalance':[450, 450]}
    )

FireflyMV = CameraTestCfg('firewire', '', -1, False,
    [CameraFormatCfg((640, 480), 'I8', 7.5),
     CameraFormatCfg((640, 480), 'I8', 15),
     CameraFormatCfg((640, 480), 'I8', 60),
     CameraFormatCfg((640, 480), 'I16', 7.5),
     CameraFormatCfg((640, 480), 'I16', 15),
     CameraFormatCfg((640, 480), 'I16', 30)],
    CameraFormatCfg((640, 480), 'I8', 30),
    [ParamTestCfg('gain', (16, 30, 64), 15, 15),
     ParamTestCfg('shutter', (1, 150, 531), 50, 20),
     ParamTestCfg('brightness', (1, 130, 255), 10, 10)],
    {'gain':16, 'shutter':100, 'brightness':130}
    )

Dragonfly2 = CameraTestCfg('firewire', '', -1, False,
    [CameraFormatCfg((640,480), 'RGB', 30),
     CameraFormatCfg((1024,768), 'I8', 15),
     CameraFormatCfg((1024,768), 'I16', 15),
     CameraFormatCfg((1024,768), 'BAYER8', 30),
     CameraFormatCfg((1024,768), 'YUV422', 7.5)],
    CameraFormatCfg((123,456), 'RGB', 30),
    [ParamTestCfg('gain', (0, 346, 683), 10, 30),
     ParamTestCfg('camgamma', (0, 1278, 4095), 30, 50),
     ParamTestCfg('brightness', (0, 127, 254), 8, 8)],
    {'gain':300, 'shutter':220, 'saturation':1200, 'camgamma':1200,
     'brightness':120, 'setWhitebalance':[450, 450]}
    )

Firei = CameraTestCfg('firewire', '', -1, False,
    [CameraFormatCfg((640,480), 'YUV411', 30), 
     CameraFormatCfg((640,480), 'RGB', 15),
     CameraFormatCfg((640,480), 'YUV422', 15),
     CameraFormatCfg((640,480), 'I8', 30),
     CameraFormatCfg((320,240), 'YUV422', 7.5)],
    CameraFormatCfg((123,456), 'RGB', 30),
     #To check: there is something strange with shutter and gain in this camera.
    [ParamTestCfg('gain', (1, 100, 255), 10, 30),
     ParamTestCfg('brightness', (128, 255, 383), 8, 8),
     ParamTestCfg('shutter', (4, 4, 4), 8, 8)],
    {'gain':87, 'shutter':6, 'brightness':304, 'setWhitebalance':[95, 87]}
    )

QuickCamProLinux = CameraTestCfg('video4linux', '', -1, False,
    [CameraFormatCfg((352,288), 'YUYV422', 30),
     CameraFormatCfg((320,240), 'YUYV422', 15),
     CameraFormatCfg((176,144), 'YUYV422', 30),
     CameraFormatCfg((640,480), 'YUYV422', 30)],
    CameraFormatCfg((123,456), 'I16', 30),
    [ParamTestCfg('brightness', (0, 127, 254), 40, 50)],
    {'brightness':-1}
    )

QuickCamPro9Win = CameraTestCfg('directshow', '', -1, False,
    [CameraFormatCfg((352,288), 'YUYV422', 30),
     CameraFormatCfg((320,240), 'YUYV422', 15),
     CameraFormatCfg((640,480), 'YUYV422', 30),
     CameraFormatCfg((176,144), 'YUYV422', 30)],
    CameraFormatCfg((123,456), 'RGB', 30),
    [ParamTestCfg('brightness', (0, 127, 254), 40, 50)],
    {'brightness': 60}
    )

QuickCamProBGRWin = CameraTestCfg('directshow', '', -1, False,
    [CameraFormatCfg((352,288), 'BGR', 30),
     CameraFormatCfg((320,240), 'BGR', 15),
     CameraFormatCfg((640,480), 'BGR', 30),
     CameraFormatCfg((176,144), 'BGR', 30)],
    CameraFormatCfg((123,456), 'YUYV422', 30),
    [ParamTestCfg('brightness', (0, 127, 254), 40, 50)],
    {'brightness': 60}
    )

