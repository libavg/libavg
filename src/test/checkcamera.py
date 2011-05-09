#!/usr/bin/env python
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
# Original author of this file is Robert Parcus <betoparcus@gmail.com>
#

from libavg import *
import argparse
import traceback


g_player = avg.Player.get()

class   CameraTest(AVGApp):
    
    def init(self):  
        self.testNumber = 0   
        self.bitMaps = []
        validCameras = ('dragonfly', 'firefly')
        parser = argparse.ArgumentParser(description=
                'A test to check camera features')
        parser.add_argument('-c', '--camera', dest='camera', action='store',
        type=str, choices=validCameras,
        help = 'Select which camera model to test (Supported: Dragonfly, Firefly)')    
        args = parser.parse_args()
        #args.camera = 'firefly'   
        #args.camera = 'dragonfly'     
        if args.camera == 'dragonfly':
            self.dragonfly2()       
        if args.camera == 'firefly':
            self.firefly()
        if args.camera is None:
            parser.print_help()
            g_player.stop() 
    
    def dragonfly2(self):
        self.value = {'gain': [0, 346, 683], 'shutter': [0, 351, 704], 
                'saturation': [0, 3700, 4095], 'camgamma': [0, 1278, 4095], 
                'brightness': [0, 127, 254], 'setwhitebalance': 
                [[1,1], [511, 511], [1023, 1023]], 
                'default': dict(gain=300, shutter=220, saturation=1200, camgamma=1200,
                brightness=120, setWhitebalance=[450, 450])}
        self.testValues = [('gain', 10, 30), ('camgamma',30 ,50),
                ('saturation', 1, 1), ('brightness', 6, 6), ('shutter', 20, 20),
                ('setwhitebalance',1 ,1)]
        #driver, device, unit, fw809, framerate, capturewidth, captureheight,
        #pixelformat, parent
        self.formats = [
                ('firewire', '', -1, False, 30, 320, 240, 'YUYV422', self._parentNode),
                ('firewire', '', -1, False, 30, 1024, 768, 'I8', self._parentNode), 
                ('firewire', '', -1, False, 7.5, 1024, 768, 'I8', self._parentNode),
                ('firewire', '', -1, False, 15, 1024, 768, 'I16', self._parentNode), 
                ('firewire', '', -1, False, 30, 320, 240, 'YUV422', self._parentNode),
                ('firewire', '', -1, False, 15, 800, 600, 'YUV422', self._parentNode),
                ('firewire', '', -1, False, 7.5, 1024, 768, 'YUYV422', self._parentNode),
                ('firewire', '', -1, False, 7.5, 1024, 768, 'YUV411', self._parentNode),
                ('firewire', '', -1, False, 30, 640, 480, 'RGB', self._parentNode)]
        self.cmds = []
        self.cmdsBuilder()
        self.looper()
    
    def firefly(self):
        self.value = {'gain': [16, 30, 64], 'shutter': [1, 150, 531],  
                'brightness': [1, 130, 255], 'default': 
                dict(gain=16, shutter=100, brightness=130)}
        self.testValues = [('gain', 15, 15), ('shutter', 50, 20),('brightness', 10, 10)]
        self.formats = [
                ('firewire', '', -1, False, 7.5, 123, 480, 'I8', self._parentNode),
                ('firewire', '', -1, False, 15, 640, 480, 'I8', self._parentNode),
                ('firewire', '', -1, False, 7.5, 640, 480, 'I16', self._parentNode),
                ('firewire', '', -1, False, 15, 640, 480, 'I16', self._parentNode),
                ('firewire', '', -1, False, 60, 640, 480, 'I8', self._parentNode),
                ('firewire', '', -1, False, 30, 640, 480, 'I16', self._parentNode),
                ('firewire', '', -1, False, 30, 640, 480, 'I8', self._parentNode)]
        self.cmds = []
        self.cmdsBuilder()
        self.looper()  
        
    def looper(self):
        time = 0
        for i in self.formats:
            set = lambda i=i: self.setCamera(i)
            if i is self.formats[-1]:
                run = lambda cmds = self.cmds: self.runCameraTest(cmds)
                g_player.setTimeout(time, set)
                g_player.setTimeout(time, run)  
            else:    
                g_player.setTimeout(time, set)
                g_player.setTimeout(time + 2000, self.destroyCamera)
                time += 4000
    
    def setCamera(self, format):
        try:
            self.cam = avg.CameraNode(driver=format[0], device=format[1], unit=format[2],
                    fw800=format[3], framerate=format[4], capturewidth=format[5],
                    captureheight=format[6], pixelformat=format[7], parent=format[8])
            self.cam.play()
            if format is not self.formats[-1]:
                try:
                    self.frameTest(format)
                except:
                    pass
                    
        except:
            print "Settings test FAILED. Settings: "+str(format[:8])
            traceback.print_exc()
            
    def frameTest(self, format):
            self.runCameraTest((lambda: self.setDefault(self.value['default']),
                    None, None, None, None, None, None, 
                    lambda a=format: self.frameTestOk(a), None))
            
    def frameTestOk(self, format):
            print "Frames are changing with settings:"+str(format[:8])
    
    def runCameraTest(self, actions):
        self.actions = actions
        self.curFrame = 0
        self.next_ID = g_player.setOnFrameHandler(self.nextAction)
        self.lastCameraFrame = -1  
    
    def nextAction(self):
        if self.cam.framenum != self.lastCameraFrame:
            #print self.curFrame, self.lastCameraFrame, self.cam.framenum
            self.lastCameraFrame += 1
            if len(self.actions) == self.lastCameraFrame:
                g_player.clearInterval(self.next_ID)
                pass
            else:
                action = self.actions[self.lastCameraFrame]
                if action != None:
                    action()
        self.curFrame += 1
        
    def setCamParam (self, param, value):
        if param == 'setwhitebalance':
            self.cam.setWhitebalance(*value)
        else:
            setattr(self.cam, param, value)
    
    def getCamImg (self, param):
        if param in ['saturation', 'setwhitebalance']:
            colour = []
            colour.append(self.cam.getBitmap().getRGB('r'))
            colour.append(self.cam.getBitmap().getRGB('g'))
            colour.append(self.cam.getBitmap().getRGB('b'))
            self.bitMaps.append(colour)
        else:    
            self.bitMaps.append(self.cam.getBitmap().getAvg())
        
    
    def testCamImage (self, minMedDiff, medMaxDiff, param):
        ok = False
        if param == 'saturation':
            if self.bitMaps[0][0] == self.bitMaps[0][1] == self.bitMaps[0][2] and\
                    self.bitMaps[0][1] < self.bitMaps[2][1] and\
                    self.bitMaps[2][1] > self.bitMaps[1][2] > self.bitMaps[2][2]:
                ok = True  
        elif param == 'setwhitebalance':
            if self.bitMaps[0][0] < self.bitMaps[1][0] < self.bitMaps[2][0] and\
                    self.bitMaps[0][1] > self.bitMaps[1][1] > self.bitMaps[2][1] and\
                    self.bitMaps[0][2] < self.bitMaps[1][2] < self.bitMaps[2][2]:
                ok = True       
        elif self.bitMaps[0]+minMedDiff < self.bitMaps[1] and\
                    self.bitMaps[1]+medMaxDiff < self.bitMaps[2]:
            ok = True
        if ok:    
            print param+"test: PASSED"
        else:
            print param+"test: FAILED (Bitmaps are too similar)"
            print str(self.bitMaps) 
        #self.writeAverage(param)
        del self.bitMaps [:]
                             
    def setDefault(self, value):
        for param in value.keys():
            if param == "setWhitebalance":
                self.cam.setWhitebalance(*value[param])
            else:
                setattr(self.cam, param, value[param])
    
    def destroyCamera(self):
        try:
            self.cam.unlink(True)
            avg.CameraNode.resetFirewireBus()
            print"camera unlinked"
        except:
            print "no camera to unlink"
        
    def cmdsBuilder(self):
        for param, minMedDiff, medMaxDiff in self.testValues:
                self.cmds.append(lambda: self.setDefault(self.value['default']))
                self.cmds.append(None)
                for x in range(3):
                    self.cmds.append(lambda param=param, x=x: 
                            self.setCamParam(param, self.value[param][x]))
                    self.cmds.append(None)
                    self.cmds.append(None)   #These seem to fix 'shutterTest' problems
                    self.cmds.append(None)
                    self.cmds.append(lambda param=param: self.getCamImg(param))
                    self.cmds.append(None)
                self.cmds.append(lambda a=minMedDiff, b=medMaxDiff, c=param:
                            self.testCamImage(a, b, c))
                self.cmds.append(None) 
        self.cmds.append(lambda: self.setDefault(self.value['default']))
        self.cmds.append(g_player.stop)
        
if __name__=='__main__':
    CameraTest.start(resolution=(1024,768))
    
