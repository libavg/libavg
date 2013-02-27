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

from libavg import avg, player


class Graph(avg.DivNode):
    def __init__(self, title='', getValue=None, parent=None, **kwargs):
        super(Graph, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self._getValue = getValue
        self._xSkip = 2
        self._curUsage = 0
        self.sensitive = False

        avg.RectNode(parent=self, strokewidth=0, fillopacity=0.6, fillcolor="FFFFFF",
                size=self.size)
        self._textNode0 = avg.WordsNode(parent=self, x=10, y=self.size.y - 22,
                color="000080")
        self._textNode1 = avg.WordsNode(parent=self, x=10, y=self.size.y - 39,
                color="000080")
        self._maxLineNode = avg.PolyLineNode(parent=self, color="880000")
        self._lineNode = avg.PolyLineNode(parent=self, color="008000")
        self.__graphText = avg.WordsNode(parent=self, x=10, y=0, color="000080")
        self.__graphText.text = title
        self._setup()

    def _setup(self):
        raise RuntimeError('Please overload _setup() function')

    def delete(self):
        def kill():
            self.unlink()
        avg.LinearAnim(self, "opacity", 200, 1., 0., False, None, kill).start()


class AveragingGraph(Graph):

    def __init__(self, title='', getValue=None, parent=None, **kwargs):
        super(AveragingGraph, self).__init__(title, getValue, parent, **kwargs)
        self.registerInstance(self, None)

    def delete(self):
        super(AveragingGraph, self).delete()
        player.clearInterval(self.__interval)
        self.__interval = None

    def _setup(self):
        self.__interval = player.setInterval(1000, self._nextMemSample)
        self.__numSamples = 0
        self._usage = [0]
        self._maxUsage = [0]
        self._minutesUsage = [0]
        self._minutesMaxUsage = [0]

    def _nextMemSample(self):
        curUsage = self._getValue()
        self._usage.append(curUsage)
        maxUsage = self._maxUsage[-1]

        if curUsage > maxUsage:
            maxUsage = curUsage
            lastMaxChangeTime = time.time()
            self._textNode1.text = ("Last increase in maximum: "
                    + time.strftime("%d.%m.%Y %H:%M:%S",
                    time.localtime(lastMaxChangeTime)))
        self._maxUsage.append(maxUsage)
        self.__numSamples += 1

        if self.__numSamples % 60 == 0:
            lastMinuteAverage = sum(self._usage[-60:]) / 60
            self._minutesUsage.append(lastMinuteAverage)
            self._minutesMaxUsage.append(maxUsage)

        if self.__numSamples < 60 * 60:
            self._plotLine(self._usage, self._lineNode, maxUsage)
            self._plotLine(self._maxUsage, self._maxLineNode, maxUsage)
        else:
            self._plotLine(self._minutesUsage, self._lineNode, maxUsage)
            self._plotLine(self._minutesMaxUsage, self._maxLineNode, maxUsage)

        self._textNode0.text = ("Max. memory usage: %(size).2f MB" %
                {"size": maxUsage / (1024 * 1024.0)})

        if self.__numSamples % 3600 == 0:
            del self._usage[0:3600]
            del self._maxUsage[0:3599]
            if self.__numSamples == 604800:
                self.__numSamples == 0

    def _plotLine(self, data, node, maxy):
        yfactor = (self.size.y - 10.0) / float(maxy)
        xfactor = (self.size.x - 10.0) / float(len(data) - 1)
        node.pos = [(pos[0] * xfactor + 10, (maxy - pos[1]) * yfactor + 10.0)
                    for pos in enumerate(data)]


class SlidingGraph(Graph):
    def __init__(self, title, getValue, limit=120.0, parent=None, **kwargs):
        super(SlidingGraph, self).__init__(title, getValue, parent, **kwargs)
        self.registerInstance(self, None)
        self._limitValue = float(limit)

    def delete(self):
        super(SlidingGraph, self).delete()
        player.clearInterval(self.__frameHandlerID)
        self.__interval = None

    def _setup(self):
        self.__frameHandlerID = player.subscribe(avg.Player.ON_FRAME, 
                self._nextFrameTimeSample)
        self._numSamples = 0
        self._lastCurUsage = 0
        self._maxFrameTime = 0
        self._values = []

    def _nextFrameTimeSample(self):
        val = self._frameTimeSample()
        self._appendValue(val)
        self._numSamples += 1

    def _appendValue(self, value):
        maxValue = min(self._limitValue, value)
        y = self.height - (self.height * (maxValue / self._limitValue))
        y = max(0, y)
        numValues = int(self.width / self._xSkip)
        self._values = (self._values + [y])[-numValues:]
        self._plotGraph()

    def _frameTimeSample(self):
        frameTime = self._getValue()
        diff = frameTime - self._lastCurUsage
        if self._numSamples < 2:
            self._maxFrameTime = 0
        if diff > self._maxFrameTime:
            lastMaxChangeTime = time.time()
            self._maxFrameTime = diff
            self._textNode0.text = ("Max FrameTime: %.f" % self._maxFrameTime + " ms" +
                    "   Time: " + time.strftime("%d.%m.%Y %H:%M:%S",
                    time.localtime(lastMaxChangeTime)))

        self._lastCurUsage = frameTime
        self._textNode1.text = ("Current FrameTime: %.f" % diff + " ms")
        return diff

    def _plotGraph(self):
        self._lineNode.pos = self._getCoords()

    def _getCoords(self):
        return zip(xrange(0, len(self._values) * self._xSkip, self._xSkip), self._values)
