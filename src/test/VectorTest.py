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
              <div id="canvas" x="0" y="0" width="160" height="120"/>
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
            for i in xrange(500):
                y = i+2.5
                line = Player.createNode("line", {"x1":2, "y1":y, "x2":10, "y2":y})
                canvas.appendChild(line)
                
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addLines,
                 lambda: self.compareImage("testlotsoflines", False), 
                ))

    def testTexturedLine(self):
        def addLine():
            line = Player.createNode("line", {"x1":2, "y1":20, "x2":100, "y2":20,
                    "texhref":"rgb24-64x64.png", "strokewidth":30})
            canvas.appendChild(line)
        def removeLine():
            self.line = canvas.getChild(0)
            self.line.unlink()
        def reAddLine():
            canvas.appendChild(self.line)
        def moveTexture():
            self.line.texcoord1 = -0.5
            self.line.texcoord2 = 1.5
        canvas = self.makeEmptyCanvas()
        addLine()
        self.start(None,
                (lambda: self.compareImage("testtexturedline1", False), 
                 removeLine,
                 lambda: self.compareImage("testtexturedline2", False), 
                 addLine,
                 lambda: self.compareImage("testtexturedline1", False), 
                 removeLine,
                 lambda: self.compareImage("testtexturedline2", False), 
                 reAddLine,
                 lambda: self.compareImage("testtexturedline1", False),
                 moveTexture,
                 lambda: self.compareImage("testtexturedline3", False)
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
        def addRect():
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
        def addRect2():
            rect = Player.createNode("rect",
                    {"x":60, "y":2, "width":50, "height":30, "fillopacity":1, 
                     "strokewidth":2})
            rect.color = "FFFF00"
            canvas.insertChild(rect, 0)
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addRect,
                 lambda: self.compareImage("testRect1", False),
                 moveRect,
                 lambda: self.compareImage("testRect2", False),
                 rotateRect,
                 lambda: self.compareImage("testRect3", False),
                 addRect2,
                 lambda: self.compareImage("testRect4", False),
                ))

    def testTexturedRect(self):
        def addRect():
            rect = Player.createNode("rect",
                    {"x":20, "y":20, "width":50, "height":40, "fillopacity":1, 
                     "strokewidth":20, "texhref":"rgb24-64x64.png"})
            canvas.appendChild(rect)
        def setTexCoords():
            rect = canvas.getChild(0)
            rect.texcoords = [-1, 0, 1, 2, 3]
        def setFillTex():
            rect = canvas.getChild(0)
            rect.filltexhref="rgb24alpha-64x64.png"
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addRect,
                 lambda: self.compareImage("testTexturedRect1", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedRect2", False),
                 setFillTex,
                 lambda: self.compareImage("testTexturedRect3", False)
                ))

    def testCurve(self):
        def addCurve():
            curve = Player.createNode("curve",
                {"x1":10.5, "y1":10, "x2":10.5, "y2":80, 
                 "x3":80.5, "y3":80, "x4":80.5, "y4":10})
            canvas.appendChild(curve)
        def changeCurve():
            curve = canvas.getChild(0)
            curve.strokewidth = 20
            curve.color="FFFF00"
        def moveCurve():
            curve = canvas.getChild(0)
            curve.pos2 = (10.5, 120)
            curve.pos3 = (80.5, 120)
        def addCurve2():
            curve = Player.createNode("curve",
                {"x1":30.5, "y1":10, "x2":30.5, "y2":120, 
                 "x3":100.5, "y3":120, "x4":100.5, "y4":10})
            curve.color="FF0000"
            canvas.appendChild(curve)
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addCurve,
                 lambda: self.compareImage("testCurve1", False),
                 changeCurve,
                 lambda: self.compareImage("testCurve2", False),
                 moveCurve,
                 lambda: self.compareImage("testCurve3", False),
                 addCurve2,
                 lambda: self.compareImage("testCurve4", False),
                )) 

    def testTexturedCurve(self):
        def addCurve():
            curve = Player.createNode("curve",
                {"x1":10.5, "y1":10, "x2":10.5, "y2":80, 
                 "x3":80.5, "y3":80, "x4":80.5, "y4":10, 
                 "strokewidth":20, "texhref":"rgb24-64x64.png"})
            canvas.appendChild(curve)
        def setTexCoords():
            curve = canvas.getChild(0)
            curve.texcoord1=-1
            curve.texcoord2=2
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addCurve,
                 lambda: self.compareImage("testTexturedCurve1", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedCurve2", False)
                )) 

    def testPolyLine(self):
        def addPolyLine():
            polyline = Player.createNode("polyline", {"strokewidth":2, "color":"FF00FF"})
            polyline.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polyline)
        def changePolyLine():
            polyline = canvas.getChild(0)
            polyline.strokewidth = 16
            polyline.color="FFFF00"
            pos = polyline.pos
            pos.append((110, 90))
            polyline.pos = pos
        def miterPolyLine():
            polyline = canvas.getChild(0)
            polyline.linejoin = "miter"
        def addPolyLine2():
            polyline = Player.createNode("polyline", {"strokewidth":2, "color":"FF00FF"})
            polyline.pos = [(110,10), (100,50), (110,70)]
            canvas.insertChild(polyline,0)
        def testEmptyPolyline():
            polyline = canvas.getChild(0)
            polyline.pos=[]
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addPolyLine,
                 lambda: self.compareImage("testPolyLine1", False),
                 changePolyLine,
                 lambda: self.compareImage("testPolyLine2", False),
                 miterPolyLine,
                 lambda: self.compareImage("testPolyLine3", False),
                 addPolyLine2,
                 lambda: self.compareImage("testPolyLine4", False),
                 testEmptyPolyline,
                 lambda: self.compareImage("testPolyLine5", False)
                ))

    def testTexturedPolyLine(self):
        def texturePolyLine():
            polyline = Player.createNode("polyline", 
                    {"strokewidth":20, "color":"FF00FF", "texhref":"rgb24-64x64.png"})
            polyline.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polyline)
        def miter():
            polyline = canvas.getChild(0)
            polyline.linejoin = "miter"
        def setTexCoords():
            polyline = canvas.getChild(0)
            polyline.texcoords = [-1, 0, 1, 2]
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (texturePolyLine,
                 lambda: self.compareImage("testTexturedPolyLine1", False),
                 miter,
                 lambda: self.compareImage("testTexturedPolyLine2", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine3", False)
                ))

    def testPolygon(self):
        def addPolygon():
            polygon = Player.createNode("polygon", {"strokewidth":2, "color":"FF00FF"})
            polygon.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polygon)
        def changePolygon():
            polygon = canvas.getChild(0)
            polygon.strokewidth = 12
            polygon.color="FFFF00"
            pos = polygon.pos
            pos.append((10, 90))
            polygon.pos = pos
        def fillPolygon():
            polygon = canvas.getChild(0)
            polygon.strokewidth = 4
            polygon.fillcolor = "00FFFF"
            polygon.fillopacity = 0.5
            pos = polygon.pos
            pos.append((80, 50))
            pos.append((50, 20))
            pos.append((40, 40))
            polygon.pos = pos
        def addEmptyPoint():
            polygon = canvas.getChild(0)
            pos = polygon.pos
            pos.append((40, 40))
            polygon.pos = pos
        def addPolygon2():
            polygon = Player.createNode("polygon", {"strokewidth":3, "color":"FF00FF"})
            polygon.pos = [(100.5,10.5), (100.5,30.5), (120.5,30.5), (120.5, 10.5)]
            canvas.insertChild(polygon, 0)
        def miterPolygons():
            polygon = canvas.getChild(0)
            polygon.linejoin = "miter"
            polygon = canvas.getChild(1)
            polygon.linejoin = "miter"
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addPolygon,
                 lambda: self.compareImage("testPolygon1", False),
                 changePolygon,
                 lambda: self.compareImage("testPolygon2", False),
                 fillPolygon,
                 lambda: self.compareImage("testPolygon3", False),
                 addEmptyPoint,
                 lambda: self.compareImage("testPolygon4", False),
                 addPolygon2,
                 lambda: self.compareImage("testPolygon5", False),
                 miterPolygons,
                 lambda: self.compareImage("testPolygon6", False)
                ))

    def testTexturedPolygon(self):
        def texturePolygon():
            polygon = Player.createNode("polygon", 
                    {"strokewidth":20, "color":"FF00FF", "texhref":"rgb24-64x64.png"})
            polygon.pos = [(10,10), (50,10), (90,50), (90, 90)]
            polygon.linejoin = "miter"
            canvas.appendChild(polygon)
        def miter():
            polygon = canvas.getChild(0)
            polygon.linejoin = "miter"
        def setTexCoords():
            polygon = canvas.getChild(0)
            polygon.texcoords = [-1, 0, 1, 2]
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (texturePolygon,
                 lambda: self.compareImage("testTexturedPolygon1", False),
                 miter,
                 lambda: self.compareImage("testTexturedPolygon2", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolygon3", False)
                ))

    def testCircle(self):
        def addCircle():
            circle = Player.createNode("circle", {"x":30, "y":30, "r":20})
            canvas.appendChild(circle)
        def changeCircle():
            circle = canvas.getChild(0)
            circle.color="FF0000"
            circle.fillcolor="FFFFFF"
            circle.fillopacity=0.5
            circle.strokewidth=3
        def textureCircle():
            circle = canvas.getChild(0)
            circle.texhref="rgb24-64x64.png"
            circle.strokewidth=20
            circle.pos = (50, 50)
            circle.texcoord1 = -1
            circle.texcoord2 = 1
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addCircle,
                 lambda: self.compareImage("testCircle1", False), 
                 changeCircle,
                 lambda: self.compareImage("testCircle2", False),
                 textureCircle,
                 lambda: self.compareImage("testCircle3", False),
                ))

def vectorTestSuite(tests):
    availableTests = (
            "testLine",
            "testLotsOfLines",
            "testLineOpacity",
            "testTexturedLine",
            "testRect",
            "testTexturedRect",
            "testCurve",
            "testTexturedCurve",
            "testPolyLine",
            "testTexturedPolyLine",
            "testPolygon",
#            "testTexturedPolygon",
            "testCircle",
            )
    return AVGTestSuite (availableTests, VectorTestCase, tests)

Player = avg.Player.get()

if __name__ == '__main__':
    runStandaloneTest (vectorTestSuite)


