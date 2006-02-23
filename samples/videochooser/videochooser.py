#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys, os, math, random, stat
# Change this to whereever libavg was installed on your computer.
sys.path.append('/usr/local/lib/python2.3/site-packages/libavg')
sys.path.append('/usr/lib/python2.4/site-packages/libavg')
import avg
import anim

maxVideoWidth = 1024
videoWidth = 80 
videoHeight = 60

def init_video_nodes():
    global numVideos
    numVideos = Player.getElementByID("main").getNumChildren()
    for i in range(numVideos):
        curNode = Player.getElementByID("video"+str(i+1))
        curNode.opacity = 0

def get_video_files():
    global numVideos
    global videoDir
    files = os.listdir(videoDir)
    numVideos = len(files)
#    if numVideos > Player.getElementByID("main").getNumChildren():
#        numVideos = Player.getElementByID("main").getNumChildren()
    curEntry = 0
    for i in range(numVideos):
        print videoDir+files[i]
        if not(stat.S_ISDIR(os.stat(videoDir+files[i]).st_mode)):
            curEntry+=1
            if curEntry <= Player.getElementByID("main").getNumChildren():
                curNode = Player.getElementByID("video"+str(curEntry))
                curNode.opacity = 1
                curNode.href = videoDir+files[i]
                curNode.play()
    numVideos = curEntry
    if numVideos >= Player.getElementByID("main").getNumChildren():
        numVideos = Player.getElementByID("main").getNumChildren()

def position_videos(offset):
    def playing(x):
        return x < 1024 and x > -videoWidth
    global numVideos
    for i in range(numVideos):
        curVideo = Player.getElementByID("video"+str(i+1))
        lastpos = curVideo.x
        curVideo.x = i*(videoWidth+20)+offset
        curVideo.y = (768-videoHeight)/2
        curVideo.width = videoWidth
        curVideo.height = videoHeight
        if playing(curVideo.x) and not(playing(lastpos)):
            curVideo.play()
        if not(playing(curVideo.x)) and playing(lastpos):
            curVideo.pause()
       
frameNum = 0

def onframe():
    global numVideo
    global frameNum
    global videoWidth
    global videoHeight
    event = Player.getMouseState()
    y = (768-event.y)-128
    if y < 0:
        y=0
    videoWidth = (y/(768.0-128))*(maxVideoWidth-80)+80
    videoHeight = videoWidth*3/4
    range = (numVideos)*(videoWidth+20)-1024+20
    offset = -(event.x*range)/1024+10
    position_videos(offset)

if len(sys.argv) < 2:
    print "Usage: videochooser.py <videodir>"
else:
    Player = avg.Player()
    Log = avg.Logger.get()
    Player.setResolution(1, 0, 0, 0) 
    Log.setCategories(Log.APP |
                      Log.WARNING | 
                      Log.PROFILE |
                      Log.PROFILE_LATEFRAMES |
                      Log.CONFIG
#                      Log.MEMORY  |
#                      Log.BLTS    
#                      Log.EVENTS
                     )
    videoDir = sys.argv[1]
    print "Using "+videoDir+" as video directory." 
    Player.loadFile("videochooser.avg")
    anim.init(Player)
    init_video_nodes()
    get_video_files()
    position_videos(-100)
    Player.setInterval(10, onframe)
    Player.setFramerate(25)
    Player.play()
