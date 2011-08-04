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
# Original author of this file is igor <igor (at) c-base (dot) org>
#

import sys, os
from libavg import avg, AVGApp, AVGAppStarter

import coordcalibrator
import apphelpers

mediadir = os.path.join(os.path.dirname(__file__), 'data')
g_Player = avg.Player.get()
g_Log = avg.Logger.get()
g_KbManager = apphelpers.KeyboardManager.get()


def camera_setup(CameraType):
    if CameraType == "Fire-i":
        paramList = [
            {'Name':"Brightness",
             'path':"/camera/brightness/@value",
             'min':128, 'max':383, 'increment':1, 'precision':0},
            {'Name':"Exposure",
             'path':"/camera/exposure/@value",
             'min':-1, 'max':511, 'increment':1, 'precision':0},
            {'Name':"Shutter",
             'path':"/camera/shutter/@value",
             'min':0, 'max':7, 'increment':1, 'precision':0},
            {'Name':"Gain",
             'path':"/camera/gain/@value",
             'min':0, 'max':255, 'increment':1, 'precision':0},
        ]
    elif CameraType == "FireFly":
        paramList = [
            {'Name':"Brightness",
             'path':"/camera/brightness/@value",
             'min':1, 'max':255, 'increment':1, 'precision':0},
            {'Name':"Shutter",
             'path':"/camera/shutter/@value",
             'min':1, 'max':533, 'increment':1, 'precision':0},
            {'Name':"Gain",
             'path':"/camera/gain/@value",
             'min':16, 'max':64, 'increment':1, 'precision':0},
            {'Name':"Gamma",
             'path':"/camera/gamma/@value",
             'min':0, 'max':1, 'increment':1, 'precision':0},
        ]
    elif CameraType == "DragonFly":
        paramList = [
            {'Name':"Brightness",
             'path':"/camera/brightness/@value",
             'min':1, 'max':255, 'increment':1, 'precision':0},
            {'Name':"Gamma",
             'path':"/camera/gamma/@value",
             'min':512, 'max':4095, 'increment':5, 'precision':0},
            {'Name':"Shutter",
             'path':"/camera/shutter/@value",
             'min':0, 'max':709, 'increment':2, 'precision':0},
            {'Name':"Gain",
             'path':"/camera/gain/@value",
             'min':16, 'max':683, 'increment':2, 'precision':0},
        ]
    else:
        g_Log.trace(g_Log.ERROR, "Unknown CameraType %s" % CameraType)
        sys.exit()

    paramList.extend([
        # Touch
        {'Name':"Threshold",
         'path':"/tracker/touch/threshold/@value",
         'min':1, 'max':255, 'increment':1, 'precision':0},
        {'Name':"Similarity",
         'path':"/tracker/touch/similarity/@value",
         'min':1, 'max':300, 'increment':1, 'precision':1},

        {'Name':"Min Area",
         'path':"/tracker/touch/areabounds/@min",
         'min':1, 'max':1000000, 'increment':3, 'precision':0},
        {'Name':"Max Area",
         'path':"/tracker/touch/areabounds/@max",
         'min':20, 'max':1000000, 'increment':10, 'precision':0},

        {'Name':"Ecc. Min",
         'path':"/tracker/touch/eccentricitybounds/@min",
         'min':1, 'max':30, 'increment':1, 'precision':1},
        {'Name':"Ecc. Max",
         'path':"/tracker/touch/eccentricitybounds/@max",
         'min':1, 'max':2000, 'increment':1, 'precision':1},

        {'Name':"Bandpass Min",
         'path':"/tracker/touch/bandpass/@min",
         'min':0, 'max':15, 'increment':.1, 'precision':1},
        {'Name':"Bandpass Max",
         'path':"/tracker/touch/bandpass/@max",
         'min':0, 'max':15, 'increment':.1, 'precision':1},
        {'Name':"Bandpass Postmult",
         'path':"/tracker/touch/bandpasspostmult/@value",
         'min':0, 'max':30, 'increment':.1, 'precision':1},

        # Track
        {'Name':"Threshold",
         'path':"/tracker/track/threshold/@value",
         'min':1, 'max':255, 'increment':1, 'precision':0},

        {'Name':"Min Area",
         'path':"/tracker/track/areabounds/@min",
         'min':1, 'max':1000000, 'increment':3, 'precision':0},
        {'Name':"Max Area",
         'path':"/tracker/track/areabounds/@max",
         'min':20, 'max':1000000, 'increment':10, 'precision':0},

        {'Name':"Ecc. Min",
         'path':"/tracker/track/eccentricitybounds/@min",
         'min':1, 'max':30, 'increment':1, 'precision':1},
        {'Name':"Ecc. Max",
         'path':"/tracker/track/eccentricitybounds/@max",
         'min':1, 'max':2000, 'increment':1, 'precision':1},

        # Transform
        {'Name':"p2",
         'path':"/transform/distortionparams/@p2",
         'min':-3, 'max':3, 'increment':0.001, 'precision':3},
        {'Name':"Trapezoid",
         'path':"/transform/trapezoid/@value",
         'min':-3, 'max':3, 'increment':0.0001, 'precision':4},
        {'Name':"Angle",
         'path':"/transform/angle/@value",
         'min':-3.15, 'max':3.15, 'increment':0.01, 'precision':2},
        {'Name':"Displ. x",
         'path':"/transform/displaydisplacement/@x",
         'min':-5000, 'max':0, 'increment':1, 'precision':0},
        {'Name':"Displ. y",
         'path':"/transform/displaydisplacement/@y",
         'min':-5000, 'max':0, 'increment':1, 'precision':0},
        {'Name':"Scale x",
         'path':"/transform/displayscale/@x",
         'min':-3, 'max':8, 'increment':0.01, 'precision':2},
        {'Name':"Scale y",
         'path':"/transform/displayscale/@y",
         'min':-3, 'max':8, 'increment':0.01, 'precision':2},
    ])
    return paramList

class Calibrator(AVGApp):
    def __init__(self, parentNode, CameraType = "FireFly", appStarter = None):
        super(Calibrator, self).__init__(parentNode)
        self.paramList = camera_setup(CameraType)
        self.parentNode=parentNode
        self.appStarter = appStarter
        self.mainNode = g_Player.createNode(
        """
        <div active="False" opacity="0">
            <image width="1280" height="800" href="black.png"/>
            <image id="cal_distorted" x="0" y="0" width="1280" height="800"
                    sensitive="false" opacity="1"/>
            <words id="cal_fps" x="30" y="30" color="00FF00" text=""/>
            <words id="cal_notification" x="390" y="390" width="500" fontsize="18"
                    font="Zurich Ex BT" color="ff3333" alignment="center" />
            <div id="cal_gui" x="30" y="540">
                <image id="cal_shadow" x="0" y="13" width="500" height="150" 
                        href="black.png" opacity="0.6"/>
                
                <words x="2" y="13" text="camera" fontsize="16" font="Zurich Ex BT" 
                        color="00FF00"/>
                <image x="2" y="32" href="CamImgBorder.png"/>
                <image id="cal_camera" x="4" y="34" width="160" height="120"/>

                <words x="168" y="13" text="nohistory" fontsize="16" font="Zurich Ex BT" 
                        color="00FF00"/>
                <image x="168" y="32" href="CamImgBorder.png"/>
                <image id="cal_nohistory" x="170" y="34" width="160" height="120"/>
                
                <words x="334" y="13" text="histogram" fontsize="16" font="Zurich Ex BT" 
                        color="00FF00"/>
                <image x="334" y="32" href="CamImgBorder.png"/>
                <image id="cal_histogram" x="336" y="34" width="160" height="120"/>

                <div id="cal_params" y="170" opacity="0.9">
                <image id="cal_shadow2" width="750" height="65" href="black.png" opacity="0.6"/>
                    <div id="cal_paramdiv0" x="2">
                        <words text="camera" y="0" fontsize="10" font="Zurich Ex BT" color="00ff00" /> 
                        <words id="cal_param0" y="12" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param1" y="24" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param2" y="36" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param3" y="48" fontsize="10" font="Zurich Ex BT" /> 
                    </div>
                    <div id="cal_paramdiv1" x="80">
                        <words text="touch" y="0" fontsize="10" font="Zurich Ex BT" color="00ff00" /> 
                        <words id="cal_param4" y="12" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param5" y="24" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param6" y="36" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param7" y="48" fontsize="10" font="Zurich Ex BT" /> 
                    </div>
                    <div id="cal_paramdiv2" x="200">
                        <words id="cal_param8" y="0" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param9" y="12" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param10" y="24" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param11" y="36" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param12" y="48" fontsize="10" font="Zurich Ex BT" /> 
                    </div>
                    <div id="cal_paramdiv3" x="350">
                        <words text="track" y="0" fontsize="10" font="Zurich Ex BT" color="00ff00" /> 
                        <words id="cal_param13" y="12" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param14" y="24" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param15" y="36" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param16" y="48" fontsize="10" font="Zurich Ex BT" /> 
                    </div>
                    <div id="cal_paramdiv4" x="500">
                        <words id="cal_param17" y="0" fontsize="10" font="Zurich Ex BT" /> 
                        <words text="distort" y="12" fontsize="10" font="Zurich Ex BT" color="00ff00" /> 
                        <words id="cal_param18" y="24" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param19" y="36" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param20" y="48" fontsize="10" font="Zurich Ex BT" /> 
                    </div>
                    <div id="cal_paramdiv5" x="650">
                        <words id="cal_param21" y="0" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param22" y="12" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param23" y="24" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param24" y="36" fontsize="10" font="Zurich Ex BT" /> 
                        <words id="cal_param25" y="48" fontsize="10" font="Zurich Ex BT" /> 
                    </div>
                </div>
            </div>
            <div id="cal_coordcalibrator" opacity="0" active="false">
              <image x="0" y="0" width="1280" height="800" href="border.png"/>
              <div id="cal_messages" x="100" y="100"/>
              <image id="cal_crosshair" href="crosshair.png"/>
              <image id="cal_feedback" href="Feedback.png"/>
            </div>
        </div>
        """)
        self.mainNode.mediadir=mediadir
        parentNode.insertChild(self.mainNode, 0)

        self.coordCal = None
        self.tracker = g_Player.getTracker()
        self.curParam = 0
        self.saveIndex = 0
        self.hideMainNodeTimeout = None
        self.video = []
        self.__guiOpacity = 1
        self.__showBigCamImage = False
        self.__notificationTimer = None
        self.__onCalibrationSuccess = None

        
        
    def _enter(self):
        
        g_KbManager.push()
        
        g_KbManager.bindKey('d', self.__trackerSetDebugImages, 'tracker set debug images')
        g_KbManager.bindKey('b', self.__bigCamImage, 'big cam image')
        g_KbManager.bindKey('up', self.__keyFuncUP, 'select parameter up')
        g_KbManager.bindKey('down', self.__keyFuncDOWN, 'select parameter down')
        g_KbManager.bindKey('left', self.__keyFuncLEFT, 'value up')
        g_KbManager.bindKey('right', self.__keyFuncRIGHT, 'value down')
        g_KbManager.bindKey('page up', self.__keyFuncPAGEUp, 'value up * 10')
        g_KbManager.bindKey('page down', self.__keyFuncPAGEDown, 'value down * 10')
        g_KbManager.bindKey('s', self.__trackerSaveConfig, 'save configuration')
        g_KbManager.bindKey('g', self.__toggleGUI, 'toggle GUI')
        g_KbManager.bindKey('c', self.__startCoordCalibration,
                'start geometry calibration')
        g_KbManager.bindKey('w', self.__saveTrackerIMG, 'SAVE trager image')
        g_KbManager.bindKey('h', self.appStarter.tracker.resetHistory, 'RESET history')
        
        self.appStarter.showTrackerImage()
        self.mainNode.active=True
        self.tracker.setDebugImages(True, True)
        avg.fadeIn(self.mainNode, 400, 1)
        Bitmap = self.tracker.getImage(avg.IMG_DISTORTED)  # Why is this needed?
        self.__onFrameID=g_Player.setOnFrameHandler(self.__onFrame)
        #grandparent = self.parentNode.getParent()
        #if grandparent:
        #    grandparent.reorderChild(grandparent.indexOf(self.parentNode), grandparent.getNumChildren()-1)
        self.displayParams()
        if self.hideMainNodeTimeout:
            g_Player.clearInterval(self.hideMainNodeTimeout)

    def _leave(self):
        #unbind all calibrator keys - bind old keys
        g_KbManager.pop()
        
        def hideMainNode():
            self.mainNode.opacity=0
            self.mainNode.active = False
        self.appStarter.hideTrackerImage()
        #grandparent = self.parentNode.getParent()
        #if grandparent:
        #    grandparent.reorderChild(grandparent.indexOf(self.parentNode), 0)
        self.hideMainNodeTimeout = g_Player.setTimeout(400, hideMainNode)
        g_Player.clearInterval(self.__onFrameID)

    def reparent(self, newParent):
        """reparents the calibrator node; returns the old(!) parent node"""
        oldParent = self.mainNode.getParent()
        self.mainNode.unlink()
        newParent.appendChild(self.mainNode)
        return oldParent

    def __deferredRefreshCB(self):
        self.displayParams()
        self.tracker.resetHistory()
        self.setNotification('')
        g_KbManager.pop()
        g_Player.getElementByID('cal_params').opacity = 0.9

    def __clearNotification(self):
        self.__notificationTimer = None
        self.setNotification('')

    def __toggleGUI(self):
        self.__guiOpacity = 1 - self.__guiOpacity
        g_Player.getElementByID('cal_gui').opacity = self.__guiOpacity

    def __onFrame(self):
        def showTrackerImage(trackerImageID, nodeID, size, pos=(0,0)):
            bitmap = self.tracker.getImage(trackerImageID)
            node = g_Player.getElementByID(nodeID)
            node.setBitmap(bitmap)
            node.size = size
            if pos != (0,0):
                node.pos = pos

            # flip:
            grid = node.getOrigVertexCoords()
            grid = [ [ (1-pos[0], pos[1]) for pos in line ] for line in grid]
            node.setWarpedVertexCoords(grid)

        if self.__showBigCamImage:
            showTrackerImage(avg.IMG_CAMERA, "cal_distorted", (1280, 960))
        else:
            pos = self.tracker.getDisplayROIPos()
            size = self.tracker.getDisplayROISize()
            showTrackerImage(avg.IMG_DISTORTED, "cal_distorted", pos = pos, size = size)
        showTrackerImage(avg.IMG_CAMERA, "cal_camera", (160, 120))
        showTrackerImage(avg.IMG_NOHISTORY, "cal_nohistory", (160, 120))
        showTrackerImage(avg.IMG_HISTOGRAM, "cal_histogram", (160, 120))
        fps = g_Player.getEffectiveFramerate()
        g_Player.getElementByID("cal_fps").text = '%(val).2f' % {'val': fps} 
        
    def __trackerSetDebugImages(self):
        self.appStarter.toggleTrackerImage()
        # toggleTrackerImage() will influence setDebugImages status, so we have to reset it:
        self.tracker.setDebugImages(True, True)
                     
    def __bigCamImage(self):
        self.__showBigCamImage = not(self.__showBigCamImage)
        
    def __keyFuncUP(self):
        if self.curParam > 0:
            self.curParam -= 1
        self.displayParams()    
                
    def __keyFuncDOWN(self):
        if self.curParam < len(self.paramList)-1:
            self.curParam += 1
        self.displayParams()  
                
    def __keyFuncLEFT(self):
        self.changeParam(-1)
        self.displayParams()  
        
    def __keyFuncRIGHT(self):
        self.changeParam(1)  
        self.displayParams()  
        
    def __keyFuncPAGEUp(self):
        self.changeParam(10)
        self.displayParams()  
        
    def __keyFuncPAGEDown(self):
        self.changeParam(-10)
        self.displayParams()  
                
    def __trackerSaveConfig(self):
        self.tracker.saveConfig()
        self.setNotification('Tracker configuration saved', 2000)
        g_Log.trace(g_Log.APP, "Tracker configuration saved.")
              
    def __saveTrackerIMG(self):
        def saveTrackerImage(id, name):
            self.tracker.getImage(id).save("img"+str(self.saveIndex)+"_"+name+".png")

        self.saveIndex += 1
        saveTrackerImage(avg.IMG_CAMERA, "camera")
        saveTrackerImage(avg.IMG_DISTORTED, "distorted")
        saveTrackerImage(avg.IMG_NOHISTORY, "nohistory")
        saveTrackerImage(avg.IMG_HIGHPASS, "highpass")
        saveTrackerImage(avg.IMG_FINGERS, "fingers")
        saveTrackerImage(avg.IMG_HISTOGRAM, "histogram")
        self.setNotification('Tracker images dumped', 2000)
        g_Log.trace(g_Log.APP, "Tracker images saved.")
    
    def __startCoordCalibration(self):
        assert(not self.coordCal)

        self.__savedShutter = self.tracker.getParam("/camera/shutter/@value")
        self.tracker.setParam("/camera/shutter/@value", "8")
        self.__savedGain = self.tracker.getParam("/camera/gain/@value")
        self.tracker.setParam("/camera/gain/@value", "16")
        self.__savedStrobe = self.tracker.getParam("/camera/strobeduration/@value")
        self.tracker.setParam("/camera/strobeduration/@value", "-1")
        self.coordCal = coordcalibrator.CoordCalibrator(self.__onCalibrationTerminated)
    
    def __onCalibrationTerminated(self, isSuccessful):
        self.coordCal = None
        self.tracker.setParam("/camera/shutter/@value", self.__savedShutter)
        self.tracker.setParam("/camera/gain/@value", self.__savedGain)
        self.tracker.setParam("/camera/strobeduration/@value", self.__savedStrobe)
        self.deferredRefresh()
        
    def setOnCalibrationSuccess(self, callback):
        self.__onCalibrationSuccess = callback

    def deferredRefresh(self):
        g_Player.setTimeout(1500, self.__deferredRefreshCB)
        self.setNotification('Please wait for settlement')
        g_KbManager.push()
        g_Player.getElementByID('cal_params').opacity = 0.3

    def setNotification(self, text, timeout=0):
        g_Player.getElementByID('cal_notification').text = text
        if timeout:
            if self.__notificationTimer is not None:
                g_Player.clearInterval(self.__notificationTimer)
            
            self.__notificationTimer = g_Player.setTimeout(timeout,
                    self.__clearNotification)

    def displayParams(self):
        i = 0
        for Param in self.paramList:
            Node = g_Player.getElementByID("cal_param"+str(i))
            Path = Param['path']
            Val = float(self.tracker.getParam(Path))
            Node.text = (Param['Name']+": "
                    +('%(val).'+str(Param['precision'])+'f') % {'val': Val})
            if self.curParam == i:
                Node.color = "FFFFFF"
            else:
                Node.color = "A0A0FF"
            i += 1 

    def changeParam(self, Change):
        param = self.paramList[self.curParam]
        if param['increment'] >= 1:
            Val = int(float(self.tracker.getParam(param['path'])))
        else:
            Val = float(self.tracker.getParam(param['path']))
        Val += Change*param['increment']
        if Val < param['min']:
            Val = param['min']
        if Val > param['max']:
            Val = param['max']
        self.tracker.setParam(param['path'], str(Val))
