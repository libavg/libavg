#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from libavg import avg
import OSC
import socket

INPUT=0
OUTPUT=1

CAMERA_TYPE="FireFly"
#CAMERA_TYPE="Fire-i"

if CAMERA_TYPE == "Fire-i":
    paramList = [
        # Camera
        {'Name':"Brightness", 
         'path':"/camera/brightness/@value", 
         'min':128, 'max':383, 'increment':1, 'precision':0},
        {'Name':"Exposure", 
         'path':"/camera/exposure/@value", 
         'min':0, 'max':511, 'increment':1, 'precision':0},
        {'Name':"Shutter", 
         'path':"/camera/shutter/@value", 
         'min':0, 'max':7, 'increment':1, 'precision':0},
        {'Name':"Gain", 
         'path':"/camera/gain/@value", 
         'min':0, 'max':255, 'increment':1, 'precision':0}
    ]
elif CAMERA_TYPE == "FireFly":
    paramList = [
        # Camera
        {'Name':"Brightness", 
         'path':"/camera/brightness/@value", 
         'min':1, 'max':255, 'increment':1, 'precision':0},
        {'Name':"Exposure", 
         'path':"/camera/exposure/@value", 
         'min':7, 'max':62, 'increment':1, 'precision':0},
        {'Name':"Shutter", 
         'path':"/camera/shutter/@value", 
         'min':1, 'max':533, 'increment':1, 'precision':0},
        {'Name':"Gain", 
         'path':"/camera/gain/@value", 
         'min':16, 'max':64, 'increment':1, 'precision':0}
    ]
else:
    print("Unknown CAMERA_TYPE")
    sys.exit()

paramList.extend([
    # Tracker
    {'Name':"Threshold", 
     'path':"/tracker/track/threshold/@value", 
     'min':1, 'max':255, 'increment':1, 'precision':0},
    {'Name':"Min Area", 
     'path':"/tracker/track/areabounds/@min", 
     'min':1, 'max':1000000, 'increment':3, 'precision':0},
    {'Name':"Max Area", 
     'path':"/tracker/track/areabounds/@max", 
     'min':20, 'max':1000000, 'increment':10, 'precision':0},
    {'Name':"Contour Precision", 
     'path':"/tracker/contourprecision/@value", 
     'min':0, 'max':1000, 'increment':1, 'precision':0},

    # Transform
    {'Name':"Displacement x", 
     'path':"/transform/displaydisplacement/@x", 
     'min':-5000, 'max':0, 'increment':1, 'precision':0},
    {'Name':"y", 
     'path':"/transform/displaydisplacement/@y", 
     'min':-5000, 'max':0, 'increment':1, 'precision':0},
    {'Name':"Scale x", 
     'path':"/transform/displayscale/@x", 
     'min':-3, 'max':8, 'increment':0.01, 'precision':2},
    {'Name':"y", 
     'path':"/transform/displayscale/@y", 
     'min':-3, 'max':8, 'increment':0.01, 'precision':2},
    {'Name':"Distortion p2", 
     'path':"/transform/distortionparams/@p2", 
     'min':-3, 'max':3, 'increment':0.001, 'precision':3},
    {'Name':"p3", 
     'path':"/transform/distortionparams/@p3", 
     'min':-3, 'max':3, 'increment':0.001, 'precision':3},
    {'Name':"Trapezoid", 
     'path':"/transform/trapezoid/@value", 
     'min':-3, 'max':3, 'increment':0.0001, 'precision':4},
    {'Name':"Angle", 
     'path':"/transform/angle/@value", 
     'min':-3.15, 'max':3.15, 'increment':0.01, 'precision':2},
])

sendContour=0
curParam=0

g_CursorDict = {}

def onTouch(Event):
    global OSCClient
    global g_TrackSize
    global g_CursorDict

    try:

        if Event.source == avg.TOUCH:
            if Event.type == avg.CURSORDOWN or Event.type == avg.CURSORMOTION:
                g_CursorDict[Event.cursorid] = Event
            elif Event.type == avg.CURSORUP:
                del g_CursorDict[Event.cursorid] 
#            if sendContour and Type != 'd':
#                bundle = OSC.Bundle()
#                bundle.append(posMsg)
#                
#                contour = Event.getContour()
#
#                # This triggers contour reset on server side
#                rstMsg = OSC.Message()
#                rstMsg.setAddress('/b/r')
#                rstMsg.append(Event.cursorid)
#                rstMsg.append(len(contour))
#                
#                bundle.append(rstMsg)
#              
#                i = 0
#                contMsg = OSC.Message()
#                for point in contour:
#                    contMsg.clear()
#                    contMsg.setAddress('/b/v')
#                    contMsg.append(Event.cursorid)
#                    contMsg.append(point[0])
#                    contMsg.append(point[1])
#                    contMsg.append(i)
#
#                    bundle.append(contMsg)
#                    i = i+1
#
#                OSCClient.sendRawMessage(bundle.getRawMessage())
#
#            else:
#            print "msg: ", posMsg
    except socket.error, e:
        print e
#        print "EVENT="+Type+" ID="+str(Event.cursorid)+" POS="+str(Event.x)+","+str(Event.y)+" AREA="+str(Event.area)

def changeParam(Change):
    global curParam
    global paramList
    param = paramList[curParam]
    if param['increment'] >= 1:
        Val = int(Tracker.getParam(param['path']))
    else:
        Val = float(Tracker.getParam(param['path']))
    Val += Change*param['increment']
    if Val < param['min']:
        Val = param['min']
    if Val > param['max']:
        Val = param['max']
    Tracker.setParam(param['path'], str(Val))
    
def displayParams():
    global paramList
    global curParam
    i = 0
    for Param in paramList:
        Node = Player.getElementByID("param"+str(i))
        Path = Param['path']
        Val = float(Tracker.getParam(Path))
        Node.text = Param['Name']+": "+('%(val).'+str(Param['precision'])+'f') % {'val': Val}
        if curParam == i:
            Node.color = "FFFFFF"
        else:
            Node.color = "A0A0FF"
        i += 1 

saveIndex=0

def onKeyUp(Event):
    global Tracker
    global showImage
    global curParam
    global saveIndex
    global paramList
    if Event.keystring == "1":
        showImage = not(showImage)
        Tracker.setDebugImages(showImage, showImage)
    elif Event.keystring == "h":
        Tracker.resetHistory()
        print "History reset"
    elif Event.keystring == "up":
        if curParam > 0:
            curParam -= 1
    elif Event.keystring == "down":
        if curParam < len(paramList)-1:
            curParam += 1
    elif Event.keystring == "left":
        changeParam(-1)
    elif Event.keystring == "right":
        changeParam(1)
    elif Event.keystring == "page up":
        changeParam(-10)
    elif Event.keystring == "page down":
        changeParam(10)
    elif Event.keystring == "h":
        Tracker.resetHistory()
        print "History reset"
    elif Event.keystring == "s":
        Tracker.saveConfig('avgtrackerrc')
        print ("Tracker configuration saved.")
    elif Event.keystring == "w":
        saveIndex += 1
        Tracker.getImage(avg.IMG_CAMERA).save("img"+str(saveIndex)+"_camera.png")
        Tracker.getImage(avg.IMG_DISTORTED).save("img"+str(saveIndex)+"_distorted.png")
        Tracker.getImage(avg.IMG_NOHISTORY).save("img"+str(saveIndex)+"_nohistory.png")
        Tracker.getImage(avg.IMG_HIGHPASS).save("img"+str(saveIndex)+"_highpass.png")
        Tracker.getImage(avg.IMG_FINGERS).save("img"+str(saveIndex)+"_fingers.png")
        Tracker.getImage(avg.IMG_HISTOGRAM).save("img"+str(saveIndex)+"_histogram.png")
        print ("Images saved.")
    displayParams()

def flipBitmap(Node):
    Grid = Node.getOrigVertexCoords()
    Grid = [ [ (pos[0], 1-pos[1]) for pos in line ] for line in Grid]
    Node.setWarpedVertexCoords(Grid)

g_FrameNum = 0

def sendFrameMsgs():
    global g_CursorDict
    aliveMsg = OSC.Message()
    aliveMsg.setAddress('/tuio/2Dcur')
    aliveMsg.append("alive")
    for id in g_CursorDict:
        aliveMsg.append(id)
    OSCClient.sendMessage(aliveMsg)
    for id in g_CursorDict:
        event = g_CursorDict[id]
        posMsg = OSC.Message()
        posMsg.setAddress('/tuio/2Dcur')
        posMsg.append("set")
        posMsg.append(event.cursorid)
        posMsg.append(float(event.x)/g_TrackSize[0])
        posMsg.append(float(event.y)/g_TrackSize[1])
        posMsg.append(event.speed[0]) # Speed
        posMsg.append(event.speed[1])
        posMsg.append(0.0) # Acceleration
        OSCClient.sendMessage(posMsg)
        print event.x, event.y, g_TrackSize
    frameMsg = OSC.Message()
    frameMsg.setAddress('/tuio/2Dcur')
    frameMsg.append("fseq")
    frameMsg.append(g_FrameNum)
    OSCClient.sendMessage(frameMsg)

def onFrame():
    def showTrackerImage(TrackerImageID, NodeID, w=None, h=None):
        global Tracker
        Bitmap = Tracker.getImage(TrackerImageID)
        Node = Player.getElementByID(NodeID)
        Node.setBitmap(Bitmap)
        if w != None:
            Node.width=w
            Node.height=h
        flipBitmap(Node)
    global showImage
    global g_FrameNum
    if showImage:
        showTrackerImage(avg.IMG_DISTORTED, "distorted", 1280, 800)
        showTrackerImage(avg.IMG_FINGERS, "fingers", 1280, 800)
        showTrackerImage(avg.IMG_CAMERA, "camera", 160, 120)
        showTrackerImage(avg.IMG_NOHISTORY, "nohistory", 160, 120)
        showTrackerImage(avg.IMG_HISTOGRAM, "histogram", 160, 120)
    fps = Player.getEffectiveFramerate()
    Player.getElementByID("fps").text = '%(val).2f' % {'val': fps} 
    sendFrameMsgs()

value = 0
Player = avg.Player()
Log = avg.Logger.get()
Player.setResolution(0, 960, 0, 0) 
#Player.setResolution(1, 0, 0, 0) 
Log.setCategories(Log.APP |
                  Log.WARNING | 
                  Log.PROFILE |
                  Log.CONFIG
#                  Log.EVENTS |
#                  Log.EVENTS2
#                  Log.PROFILE_LATEFRAMES
                 )
Player.loadFile("remote.avg")
Player.setFramerate(60)
Tracker = Player.addTracker("avgtrackerrc")
Tracker.setDebugImages(True, True)

showImage = True

#OSCClient = OSC.Client("192.168.100.113", 12000)
OSCClient = OSC.Client("127.0.0.1", 57110)
OSCClient.setBufSize(65535)
rootNode = Player.getRootNode()
g_TrackSize=(rootNode.width, rootNode.height)

#initMsg = OSC.Message()
#initMsg.setAddress('/stage/init')
#initMsg.append(320)
#initMsg.append(165)
#initMsg.append(Bitmap.getSize()[0])
#initMsg.append(Bitmap.getSize()[1])
#OSCClient.sendMessage(initMsg);

Player.setOnFrameHandler(onFrame)
displayParams()
Player.play()

