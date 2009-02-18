#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from libavg import avg
import time

def videoPlay(nodeName):
    node = AVGPlayer.getElementByID(nodeName)
    print "Starting "+nodeName
    node.play()
#    node.width=800
#    node.height=600

def rotate():
    for i in range(4):
        node = AVGPlayer.getElementByID("mpeg"+str(i+1))
        node.angle += 0.02

Log = avg.Logger.get();
Log.setCategories(Log.APP |
          Log.WARNING | 
          Log.PROFILE |
#          Log.PROFILE_LATEFRAMES |
          Log.CONFIG |
          Log.EVENTS)

AVGPlayer = avg.Player()
AVGPlayer.setResolution(0,0,0,0)

AVGPlayer.loadFile("videoperformance.avg")
def addShader(nodeName):
    node = AVGPlayer.getElementByID(nodeName)
    shaderProg="""void main() {
        gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
        gl_FrontColor = gl_Color;
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
        gl_Position.x = 2 * gl_Position.x;
    }"""
    node.setVertexShader(shaderProg)
AVGPlayer.setInterval(10, rotate)
AVGPlayer.setTimeout(1000, lambda: addShader('mpeg1'))
AVGPlayer.setTimeout(10, lambda: videoPlay('mpeg1'))
AVGPlayer.setTimeout(40, lambda: videoPlay('mpeg2'))
AVGPlayer.setTimeout(80, lambda: videoPlay('mpeg3'))
AVGPlayer.setTimeout(120, lambda: videoPlay('mpeg4'))
#AVGPlayer.setTimeout(160, lambda: videoPlay('mpeg5'))
#AVGPlayer.setTimeout(200, lambda: videoPlay('mpeg6'))
#AVGPlayer.setTimeout(24000, lambda: videoPlay('mpeg7'))
#AVGPlayer.setTimeout(28000, lambda: videoPlay('mpeg8'))
AVGPlayer.setFramerate(100)
AVGPlayer.play()

