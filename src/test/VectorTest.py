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
            line.color="FF0000"
            line.strokewidth=3
        def moveLine():
            line.pos1 += (0, 30)
            line.y2 += 30
        def blendMode():
            line = canvas.getChild(6)
            line.y1 = 7.9 
            line.y2 = 7.9 
            line.strokewidth = 10
            line.blendmode="add"
        canvas = self.makeEmptyCanvas()
        addLines()
        line = canvas.getChild(0)
        self.start(None,
                (lambda: self.compareImage("testline1", False), 
                 changeLine,
                 lambda: self.compareImage("testline2", False),
                 moveLine,
                 lambda: self.compareImage("testline3", False),
                 blendMode,
                 lambda: self.compareImage("testline4", False)
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
        def bmpTexture():
            bmp = avg.Bitmap("rgb24alpha-64x64.png")
            self.line.setBitmap(bmp)
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
                 lambda: self.compareImage("testtexturedline3", False),
                 bmpTexture,
                 lambda: self.compareImage("testtexturedline4", False)
                ))

    def testLineOpacity(self):
        def addLine():
            line = Player.createNode("line", 
                    {"x1":2, "y1":2.5, "x2":158, "y2":2.5, "opacity":0.5})
            canvas.appendChild(line)
        def changeCanvasOpacity():
            canvas.opacity = 0.5
            canvas.getChild(0).opacity = 0.25
        canvas = self.makeEmptyCanvas()
        self.start(None,
                (addLine,
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
            return rect
        def moveRect():
            rect.pos = (50, 50)
            rect.size = (30, 10)
            rect.fillcolor = "FF0000"
            rect.fillopacity = 0.5
            rect.color = "FFFF00"
            rect.strokewidth = 2
        def rotateRect():
            rect.angle = 1.57
        def addRect2():
            rect = Player.createNode("rect",
                    {"x":60, "y":2, "width":50, "height":30, "fillopacity":1, 
                     "strokewidth":2})
            rect.color = "FFFF00"
            canvas.insertChild(rect, 0)
        canvas = self.makeEmptyCanvas()
        rect = addRect()
        self.start(None,
                (lambda: self.compareImage("testRect1", False),
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
            return rect
        def setTexCoords():
            rect.texcoords = [-1, 0, 1, 2, 3]
        def setFillTex():
            rect.strokewidth = 2
            rect.texhref = ""
            rect.filltexhref="rgb24alpha-64x64.png"
        def setFillTexCoords():
            rect.filltexcoord1 = (0.5, 0.5)
            rect.filltexcoord2 = (1.5, 1.5)
        def setFillBitmap():
            bmp = avg.Bitmap("rgb24-64x64.png")
            rect.setFillBitmap(bmp)
        canvas = self.makeEmptyCanvas()
        rect = addRect()
        self.start(None,
                (lambda: self.compareImage("testTexturedRect1", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedRect2", False),
                 setFillTex,
                 lambda: self.compareImage("testTexturedRect3", False),
                 setFillTexCoords,
                 lambda: self.compareImage("testTexturedRect4", False),
                 setFillBitmap,
                 lambda: self.compareImage("testTexturedRect5", False)
                ))

    def testCurve(self):
        def addCurve():
            curve = Player.createNode("curve",
                {"x1":10.5, "y1":10, "x2":10.5, "y2":80, 
                 "x3":80.5, "y3":80, "x4":80.5, "y4":10})
            canvas.appendChild(curve)
            return curve
        def changeCurve():
            curve.strokewidth = 20
            curve.color="FFFF00"
        def moveCurve():
            curve.pos2 = (10.5, 120)
            curve.pos3 = (80.5, 120)
        def addCurve2():
            curve = Player.createNode("curve",
                {"x1":30.5, "y1":10, "x2":30.5, "y2":120, 
                 "x3":100.5, "y3":120, "x4":100.5, "y4":10})
            curve.color="FF0000"
            canvas.appendChild(curve)
        canvas = self.makeEmptyCanvas()
        curve = addCurve()
        self.start(None,
                (lambda: self.compareImage("testCurve1", False),
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
            return curve
        def setTexCoords():
            curve.texcoord1=-1
            curve.texcoord2=2
        canvas = self.makeEmptyCanvas()
        curve = addCurve()
        self.start(None,
                (lambda: self.compareImage("testTexturedCurve1", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedCurve2", False)
                )) 

    def testPolyLine(self):
        def addPolyLine():
            polyline = Player.createNode("polyline", {"strokewidth":2, "color":"FF00FF"})
            polyline.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polyline)
            return polyline
        def changePolyLine():
            polyline.strokewidth = 16
            polyline.color="FFFF00"
            pos = polyline.pos
            pos.append((110, 90))
            polyline.pos = pos
        def miterPolyLine():
            polyline.linejoin = "miter"
        def addPolyLine2():
            polyline2 = Player.createNode("polyline", {"strokewidth":2, "color":"FF00FF"})
            polyline2.pos = [(110,10), (100,50), (110,70)]
            canvas.insertChild(polyline2,0)
        def testEmptyPolyLine():
            polyline2 = canvas.getChild(0)
            polyline2.pos=[]
        def testAcutePolyLine():
            polyline2 = canvas.getChild(0)
            polyline2.strokewidth = 10
            polyline2.linejoin="bevel"
            polyline2.pos = [(50,10), (60,10), (50,11)]
            canvas.removeChild(1)
        canvas = self.makeEmptyCanvas()
        polyline = addPolyLine()
        self.start(None,
                (lambda: self.compareImage("testPolyLine1", False),
                 changePolyLine,
                 lambda: self.compareImage("testPolyLine2", False),
                 miterPolyLine,
                 lambda: self.compareImage("testPolyLine3", False),
                 addPolyLine2,
                 lambda: self.compareImage("testPolyLine4", False),
                 testEmptyPolyLine,
                 lambda: self.compareImage("testPolyLine5", False),
                 testAcutePolyLine,
                 lambda: self.compareImage("testPolyLine6", False)
                ))

    def testTexturedPolyLine(self):
        def texturePolyLine():
            polyline = Player.createNode("polyline", 
                    {"strokewidth":20, "color":"FF00FF", "texhref":"rgb24-64x64.png"})
            polyline.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polyline)
            return polyline
        def miter():
            polyline.linejoin = "miter"
        def setTexCoords():
            polyline.texcoords = [-1, 0, 1, 2]
        def repeatTexCoords():
            polyline.pos = [(10,10), (30,10), (30,50), (50,50), (50,70), (70,70)]
            polyline.texcoords = [1, 2, 3]
        canvas = self.makeEmptyCanvas()
        polyline = texturePolyLine()
        self.start(None,
                (lambda: self.compareImage("testTexturedPolyLine1", False),
                 miter,
                 lambda: self.compareImage("testTexturedPolyLine2", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine3", False),
                 repeatTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine4", False)
                ))

    def testPolygon(self):
        def addPolygon():
            polygon = Player.createNode("polygon", {"strokewidth":2, "color":"FF00FF"})
            polygon.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polygon)
            return polygon
        def changePolygon():
            polygon.strokewidth = 12
            polygon.color="FFFF00"
            pos = polygon.pos
            pos.append((10, 90))
            polygon.pos = pos
        def fillPolygon():
            polygon.strokewidth = 4
            polygon.fillcolor = "00FFFF"
            polygon.fillopacity = 0.5
            pos = polygon.pos
            pos.append((80, 50))
            pos.append((50, 20))
            pos.append((40, 40))
            polygon.pos = pos
        def addEmptyPoint():
            pos = polygon.pos
            pos.append((40, 40))
            polygon.pos = pos
        def addPolygon2():
            polygon = Player.createNode("polygon", {"strokewidth":3, "color":"FF00FF"})
            polygon.pos = [(100.5,10.5), (100.5,30.5), (120.5,30.5), (120.5, 10.5)]
            canvas.insertChild(polygon, 0)
        def miterPolygons():
            polygon.linejoin = "miter"
            polygon2 = canvas.getChild(0)
            polygon2.linejoin = "miter"
        canvas = self.makeEmptyCanvas()
        polygon = addPolygon()
        self.start(None,
                (lambda: self.compareImage("testPolygon1", True),
                 changePolygon,
                 lambda: self.compareImage("testPolygon2", True),
                 fillPolygon,
                 lambda: self.compareImage("testPolygon3", True),
                 addEmptyPoint,
                 lambda: self.compareImage("testPolygon4", True),
                 addPolygon2,
                 lambda: self.compareImage("testPolygon5", True),
                 miterPolygons,
                 lambda: self.compareImage("testPolygon6", False)
                ))

    def testTexturedPolygon(self):
        def texturePolygon():
            polygon = Player.createNode("polygon", 
                    {"strokewidth":20, "color":"FF00FF", "texhref":"rgb24-64x64.png"})
            polygon.pos = [(10,10), (50,10), (90,50), (90, 90)]
            canvas.appendChild(polygon)
            return polygon
        def miter():
            polygon.linejoin = "miter"
        def setTexCoords():
            polygon.texcoords = [-1, 0, 1, 2, 3]
        def repeatTexCoords():
            polygon.texcoords = [0, 1]
        def setFillTex():
            polygon.fillopacity=1
            polygon.texhref=""
            polygon.strokewidth=1
            polygon.filltexhref="rgb24alpha-64x64.png"
        def setFillTexCoords():
            polygon.filltexcoord1=(0.5, 1)
            polygon.filltexcoord2=(1.5, 3)
        canvas = self.makeEmptyCanvas()
        polygon = texturePolygon()
        self.start(None,
                (lambda: self.compareImage("testTexturedPolygon1", False),
                 miter,
                 lambda: self.compareImage("testTexturedPolygon2", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolygon3", False),
                 repeatTexCoords,
                 lambda: self.compareImage("testTexturedPolygon4", False),
                 setFillTex,
                 lambda: self.compareImage("testTexturedPolygon5", False),
                 setFillTexCoords,
                 lambda: self.compareImage("testTexturedPolygon6", False)
                ))

    def testCircle(self):
        def addCircle():
            circle = Player.createNode("circle", {"x":30, "y":30, "r":20})
            canvas.appendChild(circle)
            return circle
        def changeCircle():
            circle.color="FF0000"
            circle.fillcolor="FFFFFF"
            circle.fillopacity=0.5
            circle.strokewidth=3
        def textureCircle():
            circle.texhref="rgb24-64x64.png"
            circle.strokewidth=20
            circle.pos = (50, 50)
            circle.texcoord1 = -1
            circle.texcoord2 = 1
        def setFillTex():
            circle.strokewidth=1
            circle.fillopacity=1
            circle.texhref = ""
            circle.filltexhref="rgb24alpha-64x64.png"
        def setFillTexCoords():
            circle.filltexcoord1 = (0.5, 0.5)
            circle.filltexcoord2 = (1.5, 1.5)
        canvas = self.makeEmptyCanvas()
        circle = addCircle()
        self.start(None,
                (lambda: self.compareImage("testCircle1", False), 
                 changeCircle,
                 lambda: self.compareImage("testCircle2", False),
                 textureCircle,
                 lambda: self.compareImage("testCircle3", False),
                 setFillTex,
                 lambda: self.compareImage("testCircle4", False),
                 setFillTexCoords,
                 lambda: self.compareImage("testCircle5", False),
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
            "testTexturedPolygon",
            "testCircle",
            )
    return AVGTestSuite (availableTests, VectorTestCase, tests)

Player = avg.Player.get()

if __name__ == '__main__':
    runStandaloneTest (vectorTestSuite)


