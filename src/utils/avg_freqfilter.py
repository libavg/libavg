#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

import optparse
from libavg import avg
from libavg import parsecamargs
from libavg import AVGApp
from libavg.ui import simple

g_Log = avg.Logger.get()
g_Player = avg.Player.get()

parser = optparse.OptionParser()
parsecamargs.addOptions(parser)

(g_options, g_args) = parser.parse_args()

if g_options.driver is None:
    parser.print_help()
    parser.print_help()
    print
    print "ERROR: at least '--driver' must be specified"
    exit()


class Slider(avg.DivNode):
    def __init__(self, label, min, max, onChange, parent=None, **kwargs):
        super(Slider, self).__init__(**kwargs)
        if parent:
            parent.appendChild(self)
        self.__slider = simple.Slider(pos=(50,0), width=200, min=min, max=max, 
                onChange=self.__onChange, parent=self)
        avg.WordsNode(text=label, pos=(0,5), parent=self)
        self.__valNode = avg.WordsNode(text="", pos=(260,5), parent=self)
        self.__changeCallback = onChange

    def __onChange(self):
        self.__valNode.text = str(int(self.__slider.val))
        self.__changeCallback()
        
    def getVal(self):
        return self.__slider.val

    def setVal(self, val):
        self.__slider.val = val

    val = property(getVal, setVal)


class FreqFilter(AVGApp):
    def init(self):
        self.curFrame = 0
        global g_options

        self.optdict = {}
        for attr in dir(g_options):
            if attr[0] != '_':
                self.optdict[attr] = eval("g_options.%s" %attr)

        g_Log.trace(g_Log.APP, "Creating camera:")
        g_Log.trace(g_Log.APP, "driver=%(driver)s device=%(device)s" %self.optdict)
        g_Log.trace(g_Log.APP,
                "width=%(width)d height=%(height)d pixelformat='I8'" 
                %self.optdict)
        g_Log.trace(g_Log.APP, "unit=%(unit)d framerate=%(framerate)d fw800=%(fw800)s"
                %self.optdict)
           
        self.camNode = avg.CameraNode(driver=g_options.driver, device=g_options.device,
                unit=g_options.unit, fw800=g_options.fw800, framerate=g_options.framerate,
                capturewidth=g_options.width, captureheight=g_options.height,
                pixelformat="I8",
                pos=(10,10), size=(320,240), parent=self._parentNode)
        self.camNode.play()

        self.freqBPNode = avg.ImageNode(pos=(340,10), size=(320,240), 
                parent=self._parentNode)
        self.bpNode = avg.ImageNode(pos=(10, 260), size=(640, 480),
                parent=self._parentNode)

        self.rightBox = avg.DivNode(pos=(680,10), parent=self._parentNode)
        self.lowFreqSlider = Slider(label="low", min=0, max=50, 
                onChange=self.__onLowFreqChange, parent=self.rightBox)
        self.highFreqSlider = Slider(pos=(0, 50), label="high", min=0, max=50, 
                onChange=self.__onHighFreqChange, parent=self.rightBox)
        self.ampSlider = Slider(pos=(1, 90), label="amp", min=1, max=50, 
                onChange=self.__onAmpChange, parent=self.rightBox)

        size = (g_options.width, g_options.height)
        self.filter = avg.FreqFilter(size)
        self.frequencies = [10,20]
        self.amp = [1,1]
        self.filter.setFrequencies(self.frequencies, self.amp)
        self.lowFreqSlider.val = self.frequencies[0]
        self.highFreqSlider.val = self.frequencies[1]
        self.ampSlider.val = self.amp[0]
        g_Player.setOnFrameHandler(self.__onFrame)
        g_Player.setTimeout(100, self.checkCamera)

    def checkCamera(self):
        if not(self.camNode.isAvailable()):
            g_Log.trace(g_Log.APP, "Could not open camera")
            exit(1)

    def __onFrame(self):
        srcBmp = self.camNode.getBitmap()
        self.filter.filterImage(srcBmp)
        freqBPBmp = self.filter.getFreqBPImage(1)
        self.freqBPNode.setBitmap(freqBPBmp)
        bpBmp = self.filter.getBandpassImage(1)
        self.bpNode.setBitmap(bpBmp)

    def __onLowFreqChange(self):
        self.frequencies[0] = self.lowFreqSlider.val
        self.filter.setFrequencies(self.frequencies, self.amp)

    def __onHighFreqChange(self):
        self.frequencies[1] = self.highFreqSlider.val
        self.filter.setFrequencies(self.frequencies, self.amp)

    def __onAmpChange(self):
        self.amp = [self.ampSlider.val, self.ampSlider.val]
        self.filter.setFrequencies(self.frequencies, self.amp)


FreqFilter.start(resolution=(1024, 768))

