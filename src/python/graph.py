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

import time

from libavg import avg

g_Player = avg.Player.get()

class Graph(object):
    def __init__(self, graph, getValue):
        self._getValue = getValue    
        self._xSkip = 2     
        self._memGraphStartTime = g_Player.getFrameTime()
        self._curUsage = 0
         
        self._rootNode = g_Player.getRootNode()
        size = avg.Point2D(self._rootNode.width-20, self._rootNode.height/6)
        
        self._node = g_Player.createNode("""
            <div opacity="0" sensitive="False" x="10" y="10" size="%(size)s"> 
                <rect strokewidth="0" fillopacity="0.6" fillcolor="FFFFFF" 
                        size="%(size)s"/>
                <words x="10" y="%(wordsheight0)i" color="000080"/>
                <words x="10" y="%(wordsheight1)i" color="000080"/>
                <polyline color="008000"/>
                <polyline color="000080"/>
                <words x="10" y="0" color="000080"/>
            </div>""" 
            % {'size': str(size), 'wordsheight0':size.y-22, 'wordsheight1':size.y-39})
        
        self._graphSize = size-avg.Point2D(20, 20)
        self._rootNode.appendChild(self._node)
        self._textNode0 = self._node.getChild(1)
        self._textNode1 = self._node.getChild(2)
        self._maxLineNode = self._node.getChild(3)
        self._lineNode = self._node.getChild(4)
        self.__graphText = self._node.getChild(5)
        self.__graphText.text = graph

        self._setup()
        avg.fadeIn(self._node, 300)
        
    def _setup(self):
        raise RuntimeError, 'Please overload _setup() function'
    
    def setYpos(self,ypos):
        self._node.y = ypos

    def delete(self):
        def kill():
            self._node.unlink()
        avg.LinearAnim(self._node, "opacity", 300, 1, 0, None, kill).start()
        g_Player.clearInterval(self._interval)
        self._interval = None
     

class MemGraph(Graph):
    def _setup(self):
        self._interval = g_Player.setInterval(1000, self._nextMemSample)
        self._memSampleNum = 0
        self._usage = [0]
        self._maxUsage = [0]
        self._minutesUsage = [0]
        self._minutesMaxUsage = [0]
        
    def _nextMemSample(self):
        curUsage = self._getValue()
        self._usage.append(curUsage)
        maxUsage = self._maxUsage[-1]
        
        if curUsage>maxUsage:
            maxUsage = curUsage
            lastMaxChangeTime = time.time()
            self._textNode1.text = ("Last increase in maximum: "
                    +time.strftime("%d.%m.%Y %H:%M:%S", 
                        time.localtime(lastMaxChangeTime)))
        self._maxUsage.append(maxUsage)
        self._memSampleNum += 1
        
        if self._memSampleNum % 60 == 0:
            lastMinuteAverage = sum(self._usage[-60:])/60
            self._minutesUsage.append(lastMinuteAverage)
            self._minutesMaxUsage.append(maxUsage)
            
        if self._memSampleNum < 60*60:
            self._plotLine(self._usage, self._lineNode, maxUsage)
            self._plotLine(self._maxUsage, self._maxLineNode, maxUsage)              
        else:             
            self._plotLine(self._minutesUsage, self._lineNode, maxUsage)
            self._plotLine(self._minutesMaxUsage, self._maxLineNode, maxUsage)
            
        self._textNode0.text = ("Max. memory usage: %(size).2f MB"
                %{"size":maxUsage/(1024*1024.)})
        
        if self._memSampleNum % 3600 == 0:
            del self._usage[0:3600]
            del self._maxUsage[0:3599]
            if self._memSampleNum == 604800:
                self._memSampleNum == 0
                       
    def _plotLine(self, data, node, maxy):
        yfactor = self._graphSize.y/float(maxy)
        xfactor = self._graphSize.x/float(len(data)-1)
        node.pos = [(pos[0]*xfactor+10, (maxy-pos[1])*yfactor+10) 
                for pos in enumerate(data)]


class FrameRateGraph(Graph):
    def _setup(self):
        self._interval = g_Player.setOnFrameHandler(self._nextFrameTimeSample) 
        self._sampleNum = 0        
        self._memSampleNum = 0
        self._lastCurUsage = 0
        self._maxFrameTime = 0
        self._values = []
        
    def _nextFrameTimeSample(self):       
        val = self._frameTimeSample()
        self._appendValue(val)
        self._sampleNum += 1
        
    def _appendValue(self,value):
        y = value + self._rootNode.height/6
        numValues = int(self._rootNode.width/self._xSkip)-10    
        self._values = (self._values + [y])[-numValues:]
        self._plotGraph()
        
    def _frameTimeSample(self):
        frameTime = self._getValue()  
        diff = frameTime - self._lastCurUsage       
        #if(self._sampleNum % 1800 == 0):
           # self._maxFrameTime = 0
        if self._sampleNum < 2:
            self._maxFrameTime = 0
        if diff > self._maxFrameTime:
            lastMaxChangeTime = time.time()     
            self._maxFrameTime = diff
            self._textNode0.text = ("Max FrameTime: %.f" %self._maxFrameTime + " ms" + 
                "   Time: " +time.strftime("%d.%m.%Y %H:%M:%S",
                time.localtime(lastMaxChangeTime)))
        if diff > self._node.y-1:
            y = self._node.y-1
            
        self._lastCurUsage = frameTime
        self._textNode1.text = ("Current FrameTime: %.f" %diff + " ms" )      
        return -diff

    def _plotGraph(self):
        self._lineNode.pos = self._getCoords()
        
    def _getCoords(self):
        return zip(xrange(10,len(self._values)*self._xSkip, self._xSkip), self._values)
