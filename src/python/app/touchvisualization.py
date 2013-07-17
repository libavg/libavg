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

import os

from libavg import avg, player


class BaseTouchVisualization(avg.DivNode):

    def __init__(self, event, parent=None, **kwargs):
        avg.DivNode.__init__(self, **kwargs)
        self.registerInstance(self, parent)

        event.contact.subscribe(avg.Contact.CURSOR_MOTION, self._onMotion)
        event.contact.subscribe(avg.Contact.CURSOR_UP, self._onUp)
        self.pos = avg.Point2D(event.pos)
        self._fingerSize = 7*player.getPixelsPerMM() # Assume 14mm width for a finger.
        self._radius = self._getRadius(event)

    def _abort(self):
        self.unlink(True)
        del self

    def _onMotion(self, event):
        self.pos = event.pos
        self._radius = self._getRadius(event)

    def _onUp(self, event):
        self.unlink(True)
        del self

    def _getRadius(self, event):
        if event.source in [avg.Event.MOUSE]:
            return self._fingerSize
        else:
            return max(self._fingerSize, event.majoraxis.getNorm())



class DebugTouchVisualization(BaseTouchVisualization):

    def __init__(self, event, **kwargs):
        BaseTouchVisualization.__init__(self, event, **kwargs)
        self.positions = [event.pos]

        if event.source == avg.Event.TOUCH:
            color = 'e5d8d8'
        else:
            color = 'd8e5e5'
            self.opacity = 0.5
            
        self.__transparentCircle = avg.CircleNode(r=self._radius+20, fillcolor=color,
                fillopacity=0.2, opacity=0.0, strokewidth=1, sensitive=False, parent=self)
        self.__pulsecircle = avg.CircleNode(r=self._radius, fillcolor=color, color=color,
                fillopacity=0.5, opacity=0.5, strokewidth=1,
                sensitive=False, parent=self)
        if event.source in [avg.Event.TOUCH, avg.Event.TRACK]:
            self.__majorAxis = avg.LineNode(pos1=(0,0), pos2=event.majoraxis,
                    color='FFFFFF', sensitive=False, parent=self)
            self.__minorAxis = avg.LineNode(pos1=(0,0), pos2=event.minoraxis,
                    color='FFFFFF', sensitive=False, parent=self)
        if event.source == avg.Event.TOUCH:
            self.__handAxis = avg.LineNode(pos1=(0,0), pos2=self.__getHandVector(event),
                    opacity=0.5, color='A0FFA0', sensitive=False, parent=self)
        fontPos = avg.Point2D(self.__pulsecircle.r, 0)

        if event.cursorid == -1:
            text = 'MOUSE'
        else:
            text = '%s %d' % (event.source, event.cursorid)
        avg.WordsNode(pos=fontPos, text=text, fontsize=9, parent=self)
        self.motionPath = avg.PolyLineNode(pos=self.positions,
                opacity=0.7, color=color, parent=kwargs['parent'])
        self.motionVector = avg.LineNode(pos1=(0,0) , pos2=-event.contact.motionvec,
                opacity=0.4, parent=self)
        pulseCircleAnim = avg.LinearAnim(self.__pulsecircle, 'r', 200, 50, self._radius)
        pulseCircleAnim.start()

    def __del__(self):
        if self.motionPath:
            self.motionPath.unlink(True)

    def _onMotion(self, event):
        BaseTouchVisualization._onMotion(self, event)
        self.positions.append(event.pos)
        if len(self.positions) > 100:
            self.positions.pop(0)
       
        self.__pulsecircle.r = self._radius
        self.setAxisSecondPos(event)
        self.motionVector.pos2 = -event.contact.motionvec
        if event.source == avg.Event.TOUCH:
            self.__handAxis.pos2 = self.__getHandVector(event)
        self.motionPath.pos = self.positions

    def __getHandVector(self, event):
        return -avg.Point2D.fromPolar(event.handorientation, 30)

    def setAxisSecondPos(self, event):
        if event.source not in [avg.Event.MOUSE]:
            self.__majorAxis.pos2 = event.majoraxis
            self.__minorAxis.pos2 = event.minoraxis


class TouchVisualization(BaseTouchVisualization):

    mediadir = os.path.join(os.path.dirname(__file__), os.path.pardir, 'data')
    sources = [avg.Event.TOUCH]
    bmp = avg.Bitmap(mediadir+"/TouchFeedback.png")

    def __init__(self, event, **kwargs):
        BaseTouchVisualization.__init__(self, event, **kwargs)

        if event.source in self.sources:
            self.__circle = avg.ImageNode(parent=self)
            self.__circle.setBitmap(self.bmp)
            self.__setRadius(self._radius)
            avg.LinearAnim(self.__circle, "opacity", 200, 0.7, 0.4).start()
        else:
            self.unlink(True)
            self._abort()

    def _onMotion(self, event):
        BaseTouchVisualization._onMotion(self, event)
        self.__setRadius(self._radius)

    def _onUp(self, event):
        
        def gone(self):
            BaseTouchVisualization._onUp(self, event)
            self.unlink(True)
            del self

        avg.fadeIn(self.__circle, 100, 1)
        avg.LinearAnim(self.__circle, "size", 100, self.__circle.size, (4,4)).start()
        avg.LinearAnim(self.__circle, "pos", 100, self.__circle.pos, (-2,-2)).start()
        player.setTimeout(100, lambda: gone(self))

    def __setRadius(self, radius):
        self.__circle.pos = (-radius, -radius)
        self.__circle.size = (radius*2,radius*2)
        

class TouchVisualizationOverlay(avg.DivNode):
    def __init__(self, isDebug, visClass, parent=None, **kwargs):
        super(TouchVisualizationOverlay, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.sensitive = False
        self.visClass = visClass

        rootNode = player.getRootNode()
        if isDebug:
            self.elementoutlinecolor='FFFFAA'
            avg.RectNode(parent=self, size=self.size, fillopacity=0.2, fillcolor='000000')
        rootNode.subscribe(avg.Node.CURSOR_DOWN, self.__onTouchDown)
        rootNode.subscribe(avg.Node.HOVER_DOWN, self.__onTouchDown)

    def __del__(self):
        rootNode = player.getRootNode()
        if rootNode:
            rootNode.unsubscribe(avg.Node.CURSOR_DOWN, self.__onTouchDown)
            rootNode.unsubscribe(avg.Node.HOVER_DOWN, self.__onTouchDown)

    def __onTouchDown(self, event):
        self.visClass(event, parent=self)
