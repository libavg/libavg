# -*- coding: utf-8 -*-
import unittest

import sys, time, platform

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess. 
sys.path += ['../wrapper/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg    # Under windows, there is no uninstalled version.
else:    
    import avg

from testcase import *

class VectorTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def makeEmptyCanvas(self):
        Player.loadString("""
            <?xml version="1.0"?>
            <!DOCTYPE avg SYSTEM "../../doc/avg.dtd">
            <avg width="160" height="120">
              <canvas id="canvas" x="0" y="0" width="160" height="120"/>
            </avg>
        """)
        return Player.getElementByID("canvas")

    def testLine(self):
        def addLines():
            def addLine(attribs):
                line = Player.createNode("line", attribs)
                canvas.appendChild(line)
            addLine({"x1":2, "y1":2.5, "x2":100, "y2":2.5})
            addLine({"x1":2, "y1":5, "x2":100, "y2":5, "strokewidth":2})
            addLine({"x1":2.5, "y1":20, "x2":2.5, "y2":100})
            addLine({"x1":5, "y1":20, "x2":5, "y2":100, "strokewidth":2})
            addLine({"x1":2, "y1":7.5, "x2":100, "y2":7.5, "color":"FF0000"})
            addLine({"x1":2, "y1":9.5, "x2":100, "y2":9.5, "color":"00FF00"})
            addLine({"x1":2, "y1":11.5, "x2":100, "y2":11.5, "color":"0000FF"})
        def changeLine():
            line = canvas.getChild(0)
            line.color="FF0000"
            line.strokewidth=3
        def moveLine():
            line = canvas.getChild(0)
            line.pos1 += (0, 30)
            line.y2 += 30
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addLines,
                 lambda: self.compareImage("testline1", False), 
                 changeLine,
                 lambda: self.compareImage("testline2", False),
                 moveLine,
                 lambda: self.compareImage("testline3", False),
                ))

    def testLotsOfLines(self):
        def addLines():
            for i in xrange(5000):
                y = i/10+2.5
                line = Player.createNode("line", {"x1":2, "y1":y, "x2":10, "y2":y})
                canvas.appendChild(line)
                
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addLines,
                 lambda: self.compareImage("testlotsoflines", False), 
                ))

    def testLineOpacity(self):
        def addLines():
            line = Player.createNode("line", 
                    {"x1":2, "y1":2.5, "x2":158, "y2":2.5, "opacity":0.5})
            canvas.appendChild(line)
        def changeCanvasOpacity():
            canvas.opacity = 0.5
            canvas.getChild(0).opacity = 0.25
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addLines,
                 lambda: self.compareImage("testlineopacity1", False), 
                 changeCanvasOpacity,
                 lambda: self.compareImage("testlineopacity2", False), 
                ))

    def testRect(self):
        def addRects():
            rect = Player.createNode("rect",
                    {"x":2, "y":2, "width":50, "height":30, "fillopacity":1, 
                     "strokewidth":0})
            canvas.appendChild(rect)
        def moveRect():
            rect = canvas.getChild(0)
            rect.pos = (50, 50)
            rect.size = (30, 10)
            rect.fillcolor = "FF0000"
            rect.fillopacity = 0.5
            rect.color = "FFFF00"
            rect.strokewidth = 2
        def rotateRect():
            rect = canvas.getChild(0)
            rect.angle = 1.57
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addRects,
                 lambda: self.compareImage("testRect1", False),
                 moveRect,
                 lambda: self.compareImage("testRect2", False),
                 rotateRect,
                 lambda: self.compareImage("testRect3", False)
                ))

    def testCurve(self):
        def addCurves():
            curve = Player.createNode("curve",
                {"x1":10.5, "y1":10, "x2":10.5, "y2":80, 
                 "x3":80.5, "y3":80, "x4":80.5, "y4":10})
            canvas.appendChild(curve)
        def changeCurve():
            curve = canvas.getChild(0)
            curve.strokewidth = 20
            curve.color="FFFF00"
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addCurves,
                 lambda: self.compareImage("testCurve1", False),
                 changeCurve,
                 lambda: self.compareImage("testCurve2", False),
                )) 

def vectorTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(VectorTestCase("testLine"))
    suite.addTest(VectorTestCase("testLotsOfLines"))
    suite.addTest(VectorTestCase("testLineOpacity"))
    suite.addTest(VectorTestCase("testRect"))
    suite.addTest(VectorTestCase("testCurve"))
    return suite

Player = avg.Player.get()
