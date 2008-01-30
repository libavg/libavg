#!/usr/bin/python
# -*- coding: utf-8 -*-


import sys, os, math, stat
from libavg import avg
from libavg import anim

class CamCalibrator:
    def __init__(self, Player, Tracker, CameraType, ParentNode):
        self.__Player = Player
        self.__Tracker = Tracker
        self.__curParam = 0
        self.__saveIndex = 0
        self.__onFrameID = Player.setOnFrameHandler(self.onFrame)
        self.__makeParamList(CameraType)
        self.__parentNode = ParentNode
        node = Player.createNode('<image href="black.png" opacity="0.4"/>')
        node.width = ParentNode.width
        node.height = ParentNode.height
        ParentNode.appendChild(node)
        node = Player.createNode('<image id="cc_distorted"/>')
        ParentNode.appendChild(node)
        node = Player.createNode('<image id="cc_fingers"/>')
        ParentNode.appendChild(node)
        node = Player.createNode('''
            <div id="cc_gui" x="30" y="30" >
                <image x="0" y="13" width="500" height="215" 
                        href="black.png" opacity="0.6"/>
                <words x="2" y="13" text="camera" size="16" font="Eurostile" color="00FF00"/>
                <image x="2" y="32" href="CamImgBorder.png"/>
                <image id="cc_camera" x="4" y="34" width="160" height="120"/>
                
                <words x="168" y="13" text="nohistory" size="16" font="Eurostile" color="00FF00"/>
                <image x="168" y="32" href="CamImgBorder.png"/>
                <image id="cc_nohistory" x="170" y="34" width="160" height="120"/>
                
                <words x="334" y="13" text="histogram" size="16" font="Eurostile" color="00FF00"/>
                <image x="334" y="32" href="CamImgBorder.png"/>
                <image id="cc_histogram" x="336" y="34" width="160" height="120"/>
        
                <words id="cc_param0" x="2" y="162" size="13" font="Eurostile"/> 
                <words id="cc_param1" x="2" y="178" size="13" font="Eurostile"/> 
                <words id="cc_param2" x="2" y="194" size="13" font="Eurostile"/> 
                <words id="cc_param3" x="2" y="210" size="13" font="Eurostile"/> 
                <words id="cc_param4" x="168" y="162" size="13" font="Eurostile"/> 
                <words id="cc_param5" x="168" y="178" size="13" font="Eurostile"/> 
                <words id="cc_param6" x="168" y="194" size="13" font="Eurostile"/> 
                <words id="cc_param7" x="168" y="210" size="13" font="Eurostile"/> 
                <words id="cc_param8" x="304" y="162" size="13" font="Eurostile"/> 
                <words id="cc_param9" x="428" y="162" size="13" font="Eurostile"/> 
                <words id="cc_param10" x="304" y="178" size="13" font="Eurostile"/> 
                <words id="cc_param11" x="428" y="178" size="13" font="Eurostile"/> 
                <words id="cc_param12" x="304" y="194" size="13" font="Eurostile"/> 
                <words id="cc_param13" x="428" y="194" size="13" font="Eurostile"/> 
                <words id="cc_param14" x="304" y="210" size="13" font="Eurostile"/> 
                <words id="cc_param15" x="428" y="210" size="13" font="Eurostile"/> 
            </div>
        ''')
        node.width = ParentNode.width
        node.height = ParentNode.height
        ParentNode.appendChild(node)
        self.__isActive = True
        self.__switchActive()

    def __flipBitmap(self, ImgName):
        Node = self.__player.getElementByID(ImgName)
        Grid = Node.getOrigVertexCoords()
        Grid = [ [ (pos[0], 1-pos[1]) for pos in line ] for line in Grid]
        Node.setWarpedVertexCoords(Grid)

    def __switchActive(self):
        self.__isActive = not(self.__isActive)
        if self.__isActive:
            self.__parentNode.active = 1 
            self.__parentNode.opacity = 1
            self.__displayParams()
            self.__onFrameID = self.__Player.setOnFrameHandler(self.onFrame)
        else:
            self.__parentNode.active = 0 
            self.__parentNode.opacity = 0
            self.__Player.clearInterval(self.__onFrameID)
        self.__Tracker.setDebugImages(self.__isActive, self.__isActive)

    def __makeParamList(self, CameraType):
        if CameraType == "Fire-i":
            self.__paramList = [
                # Camera
                {'Name':"Brightness", 
                 'path':"/trackerconfig/camera/brightness/@value", 
                 'min':128, 'max':383, 'increment':1, 'precision':0},
                {'Name':"Exposure", 
                 'path':"/trackerconfig/camera/exposure/@value", 
                 'min':0, 'max':511, 'increment':1, 'precision':0},
                {'Name':"Shutter", 
                 'path':"/trackerconfig/camera/shutter/@value", 
                 'min':0, 'max':7, 'increment':1, 'precision':0},
                {'Name':"Gain", 
                 'path':"/trackerconfig/camera/gain/@value", 
                 'min':0, 'max':255, 'increment':1, 'precision':0}
            ]
        elif CameraType == "FireFly":
            self.__paramList = [
                # Camera
                {'Name':"Brightness", 
                 'path':"/trackerconfig/camera/brightness/@value", 
                 'min':1, 'max':255, 'increment':1, 'precision':0},
                {'Name':"Exposure", 
                 'path':"/trackerconfig/camera/exposure/@value", 
                 'min':7, 'max':62, 'increment':1, 'precision':0},
                {'Name':"Shutter", 
                 'path':"/trackerconfig/camera/shutter/@value", 
                 'min':1, 'max':533, 'increment':1, 'precision':0},
                {'Name':"Gain", 
                 'path':"/trackerconfig/camera/gain/@value", 
                 'min':16, 'max':64, 'increment':1, 'precision':0}
            ]
        else:
            print("CamCalibrator: unknown CameraType")
            sys.exit()

        self.__paramList.extend([
            # Tracker
            {'Name':"Threshold", 
             'path':"/trackerconfig/tracker/track/threshold/@value", 
             'min':1, 'max':255, 'increment':1, 'precision':0},
            {'Name':"Min Area", 
             'path':"/trackerconfig/tracker/track/areabounds/@min", 
             'min':1, 'max':1000000, 'increment':3, 'precision':0},
            {'Name':"Max Area", 
             'path':"/trackerconfig/tracker/track/areabounds/@max", 
             'min':20, 'max':1000000, 'increment':10, 'precision':0},
            {'Name':"Contour Precision", 
             'path':"/trackerconfig/tracker/contourprecision/@value", 
             'min':0, 'max':1000, 'increment':1, 'precision':0},
        
            # Transform
            {'Name':"Displacement x", 
             'path':"/trackerconfig/transform/displaydisplacement/@x", 
             'min':-5000, 'max':0, 'increment':1, 'precision':0},
            {'Name':"y", 
             'path':"/trackerconfig/transform/displaydisplacement/@y", 
             'min':-5000, 'max':0, 'increment':1, 'precision':0},
            {'Name':"Scale x", 
             'path':"/trackerconfig/transform/displayscale/@x", 
             'min':-3, 'max':8, 'increment':0.01, 'precision':2},
            {'Name':"y", 
             'path':"/trackerconfig/transform/displayscale/@y", 
             'min':-3, 'max':8, 'increment':0.01, 'precision':2},
            {'Name':"Distortion p2", 
             'path':"/trackerconfig/transform/distortionparams/@p2", 
             'min':-3, 'max':3, 'increment':0.0000001, 'precision':7},
            {'Name':"p3", 
             'path':"/trackerconfig/transform/distortionparams/@p3", 
             'min':-3, 'max':3, 'increment':0.0000001, 'precision':7},
            {'Name':"Trapezoid", 
             'path':"/trackerconfig/transform/trapezoid/@value", 
             'min':-3, 'max':3, 'increment':0.00001, 'precision':5},
            {'Name':"Angle", 
             'path':"/trackerconfig/transform/angle/@value", 
             'min':-3.15, 'max':3.15, 'increment':0.01, 'precision':2},
        ])

    def __changeParam(self, Change):
        param = self.__paramList[self.__curParam]
        if param['increment'] >= 1:
            Val = int(self.__Tracker.getParam(param['path']))
        else:
            Val = float(self.__Tracker.getParam(param['path']))
        Val += Change*param['increment']
        if Val < param['min']:
            Val = param['min']
        if Val > param['max']:
            Val = param['max']
        self.__Tracker.setParam(param['path'], str(Val))
        
    def __displayParams(self):
        i = 0
        for Param in self.__paramList:
            Node = self.__Player.getElementByID("cc_param"+str(i))
            Path = Param['path']
            Val = float(self.__Tracker.getParam(Path))
            Node.text = Param['Name']+": "+('%(val).'+str(Param['precision'])+'f') % {'val': Val}
            if self.__curParam == i:
                Node.color = "FFFFFF"
            else:
                Node.color = "A0A0FF"
            i += 1 

    def __saveTrackerImage(self, ImageID, ImageName):
        self.__Tracker.getImage(ImageID).save(
                "img"+str(self.__saveIndex)+"_"+ImageName+".png")
    
    def onFrame(self):
        def showTrackerImage(TrackerImageID, NodeID, w=None, h=None):
            Bitmap = self.__Tracker.getImage(TrackerImageID)
            Node = self.__Player.getElementByID(NodeID)
            Node.setBitmap(Bitmap)
            if w != None:
                Node.width=w
                Node.height=h
        showTrackerImage(avg.IMG_DISTORTED, "cc_distorted", 
                self.__parentNode.width, self.__parentNode.height)
        showTrackerImage(avg.IMG_FINGERS, "cc_fingers",
                self.__parentNode.width, self.__parentNode.height)
        showTrackerImage(avg.IMG_CAMERA, "cc_camera", 160, 120)
        showTrackerImage(avg.IMG_NOHISTORY, "cc_nohistory", 160, 120)
        showTrackerImage(avg.IMG_HISTOGRAM, "cc_histogram", 160, 120)

    def onKeyUp(self, Event):
        if Event.keystring == "t":
            self.__switchActive()
            return True
        else:
            if self.__isActive:
                if Event.keystring == "up":
                    if self.__curParam > 0:
                        self.__curParam -= 1
                elif Event.keystring == "down":
                    if self.__curParam < len(self.__paramList)-1:
                        self.__curParam += 1
                elif Event.keystring == "left":
                    self.__changeParam(-1)
                elif Event.keystring == "right":
                    self.__changeParam(1)
                elif Event.keystring == "page up":
                    self.__changeParam(-10)
                elif Event.keystring == "page down":
                    self.__changeParam(10)
                elif Event.keystring == "h":
                    self.__Tracker.resetHistory()
                    print "History reset"
                elif Event.keystring == "s":
                    Tracker.saveConfig()
                    print ("Tracker configuration saved.")
                elif Event.keystring == "w":
                    self.__saveIndex += 1
                    self.__saveTrackerImage(avg.IMG_CAMERA, "camera")
                    self.__saveTrackerImage(avg.IMG_DISTORTED, "distorted")
                    self.__saveTrackerImage(avg.IMG_NOHISTORY, "nohistory")
                    self.__saveTrackerImage(avg.IMG_HIGHPASS, "highpass")
                    self.__saveTrackerImage(avg.IMG_FINGERS, "fingers")
                    print ("Images saved.")
                elif Event.keystring == "i":
                    self.__showImages()
                else:
                    return False
                self.__displayParams()
                return True
            else:
                return False

