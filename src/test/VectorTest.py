# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

from libavg import avg, player
from testcase import *

class VectorTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def makeEmptyCanvas(self):
        root = self.loadEmptyScene()
        canvas = avg.DivNode(id="canvas", size=(160,120), parent=root)
        return canvas

    def testLine(self):
        def addLines():
            def addLine(attribs):
                line = player.createNode("line", attribs)
                canvas.appendChild(line)

            addLine({"pos1":(2, 2.5), "pos2":(100, 2.5)})
            addLine({"pos1":(2, 5), "pos2":(100, 5), "strokewidth":2})
            addLine({"pos1":(2.5, 20), "pos2":(2.5, 100)})
            addLine({"pos1":(5, 20), "pos2":(5, 100), "strokewidth":2})
            addLine({"pos1":(2, 7.5), "pos2":(100, 7.5), "color":"FF0000"})
            addLine({"pos1":(2, 9.5), "pos2":(100, 9.5), "color":"00FF00"})
            addLine({"pos1":(2, 11.5), "pos2":(100, 11.5), "color":"0000FF"})

        def changeLine():
            line.color="FF0000"
            line.strokewidth=3

        def moveLine():
            line.pos1 += (0, 30)
            line.pos2 += (0, 30)
        
        def blendMode():
            line = canvas.getChild(6)
            line.pos1 = (line.pos1.x, 7.9)
            line.pos2 = (line.pos2.x, 7.9)
            line.strokewidth = 10
            line.blendmode="add"
        
        canvas = self.makeEmptyCanvas()
        addLines()
        line = canvas.getChild(0)
        self.start(False,
                (lambda: self.compareImage("testline1"), 
                 changeLine,
                 lambda: self.compareImage("testline2"),
                 moveLine,
                 lambda: self.compareImage("testline3"),
                 blendMode,
                 lambda: self.compareImage("testline4")
                ))

    def testLotsOfLines(self):
        def addLines():
            for i in xrange(500):
                y = i+2.5
                line = avg.LineNode(pos1=(2, y), pos2=(10, y))
                canvas.appendChild(line)
       
        canvas = self.makeEmptyCanvas()
        self.start(False,
                (addLines,
                 lambda: self.compareImage("testlotsoflines"), 
                ))

    def testTexturedLine(self):
        def addLine():
            line = avg.LineNode(pos1=(2, 20), pos2=(100, 20), texhref="rgb24-64x64.png",
                    strokewidth=30)
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
            bmp = avg.Bitmap("media/rgb24alpha-64x64.png")
            self.line.setBitmap(bmp)
        
        def bmpNoTexture():
            self.line.setBitmap(None)
        
        canvas = self.makeEmptyCanvas()
        addLine()
        self.start(False,
                (lambda: self.compareImage("testtexturedline1"), 
                 removeLine,
                 lambda: self.compareImage("testtexturedline2"), 
                 addLine,
                 lambda: self.compareImage("testtexturedline1"), 
                 removeLine,
                 lambda: self.compareImage("testtexturedline2"), 
                 reAddLine,
                 lambda: self.compareImage("testtexturedline1"),
                 moveTexture,
                 lambda: self.compareImage("testtexturedline3"),
                 bmpTexture,
                 lambda: self.compareImage("testtexturedline4"),
                 bmpNoTexture,
                 lambda: self.compareImage("testtexturedline5"),
                ))

    def testLineOpacity(self):
        def addLine():
            line = avg.LineNode(pos1=(2, 2.5), pos2=(158, 2.5), opacity=0.5)
            canvas.appendChild(line)
        
        def changeCanvasOpacity():
            canvas.opacity = 0.5
            canvas.getChild(0).opacity = 0.25
        
        canvas = self.makeEmptyCanvas()
        self.start(False,
                (addLine,
                 lambda: self.compareImage("testlineopacity1"), 
                 changeCanvasOpacity,
                 lambda: self.compareImage("testlineopacity2"), 
                ))

    def testRect(self):
        def addRect():
            rect = avg.RectNode(pos=(2, 2), size=(50, 30), fillopacity=1, 
                    strokewidth=0)
            canvas.appendChild(rect)
            rect.subscribe(avg.Node.CURSOR_DOWN, onMouseDown)
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
            rect = avg.RectNode(pos=(60, 2), size=(50, 30), fillopacity=1, 
                    strokewidth=2)
            rect.color = "FFFF00"
            canvas.insertChild(rect, 0)
        
        def onMouseDown(event):
            self.__mouseDownCalled = True
        
        self.__mouseDownCalled = False
        canvas = self.makeEmptyCanvas()
        rect = addRect()
        self.start(False,
                (lambda: self.compareImage("testRect1"),
                 moveRect,
                 lambda: self.compareImage("testRect2"),
                 rotateRect,
                 lambda: self.compareImage("testRect3"),
                 addRect2,
                 lambda: self.compareImage("testRect4"),
                 lambda: self.fakeClick(100, 100),
                 lambda: self.assertEqual(self.__mouseDownCalled, False),
                 lambda: self.fakeClick(55, 50),
                 lambda: self.assertEqual(self.__mouseDownCalled, False),
                 lambda: self.fakeClick(65, 65),
                 lambda: self.assert_(self.__mouseDownCalled)
                ))

    def testTexturedRect(self):
        def addRect():
            self.rect = avg.RectNode(pos=(20, 20), size=(50, 40), fillopacity=1,
                     filltexcoord1=(1,1), filltexcoord2=(0,0), strokewidth=20,
                     texcoords=(1, 0.75, 0.5, 0.25, 0), texhref="rgb24-64x64.png")
            canvas.appendChild(self.rect)
            return self.rect
        
        def newRect():
            self.rect.unlink()
            self.rect = player.createNode(
                """<rect pos="(20, 20)" size="(50, 40)" fillopacity="1"
                        filltexcoord1="(1,1)" filltexcoord2="(0,0)"
                        texcoords="(0, 0.25, 0.5, 0.75, 1)"
                        strokewidth="20" texhref="rgb24-64x64.png"/>""")
            canvas.appendChild(self.rect)
        
        def setTexCoords():
            self.rect.texcoords = [-1, 0, 1, 2, 3]
        
        def setFillTex():
            self.rect.strokewidth = 2
            self.rect.texhref = ""
            self.rect.filltexhref="rgb24alpha-64x64.png"
        
        def setFillTexCoords():
            self.rect.filltexcoord1 = (0.5, 0.5)
            self.rect.filltexcoord2 = (1.5, 1.5)
        
        def setFillBitmap():
            bmp = avg.Bitmap("media/rgb24-64x64.png")
            self.rect.setFillBitmap(bmp)
        
        def clearFillBitmap():
            self.rect.fillcolor = "FF0000"
            self.rect.setFillBitmap(None)

        def setTransparentBorder():
            self.rect.fillopacity = 0
            self.rect.texhref = "rectborder.png"
            self.rect.strokewidth = 10

        canvas = self.makeEmptyCanvas()
        addRect()
        self.start(False,
                (lambda: self.compareImage("testTexturedRect1"),
                 newRect,
                 lambda: self.compareImage("testTexturedRect2"),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedRect3"),
                 setFillTex,
                 lambda: self.compareImage("testTexturedRect4"),
                 setFillTexCoords,
                 lambda: self.compareImage("testTexturedRect5"),
                 setFillBitmap,
                 lambda: self.compareImage("testTexturedRect6"),
                 clearFillBitmap,
                 lambda: self.compareImage("testTexturedRect7"),
                 setFillBitmap,
                 lambda: self.compareImage("testTexturedRect6"),
#                 setTransparentBorder,
#                 lambda: self.compareImage("testTexturedRect8"),
                ))

    def testCurve(self):
        def addCurve():
            curve = avg.CurveNode(pos1=(10.5, 10), pos2=(10.5, 80), pos3=(80.5, 80), 
                    pos4=(80.5, 10))
            canvas.appendChild(curve)
            return curve
        
        def changeCurve():
            curve.strokewidth = 19
            curve.color="FFFF00"
        
        def moveCurve():
            curve.pos2 = (10.5, 120)
            curve.pos3 = (80.5, 120)
        
        def addCurve2():
            curve = avg.CurveNode(pos1=(30.5, 10), pos2=(30.5, 120), pos3=(100.5, 120),
                 pos4=(100.5, 10))
            curve.color="FF0000"
            canvas.appendChild(curve)
        
        canvas = self.makeEmptyCanvas()
        curve = addCurve()
        self.assertAlmostEqual(curve.length, 210)
        self.assertAlmostEqual(curve.getPtOnCurve(0), (10.5,10))
        self.assertAlmostEqual(curve.getPtOnCurve(1), (80.5,10))
        self.assertAlmostEqual(curve.getPtOnCurve(0.5), (45.5,62.5))
        self.start(False,
                (lambda: self.compareImage("testCurve1"),
                 changeCurve,
                 lambda: self.compareImage("testCurve2"),
                 moveCurve,
                 lambda: self.compareImage("testCurve3"),
                 addCurve2,
                 lambda: self.compareImage("testCurve4"),
                )) 

    def testTexturedCurve(self):
        def addCurve():
            curve = avg.CurveNode(pos1=(10.5, 10), pos2=(10.5, 80), pos3=(80.5, 80), 
                 pos4=(80.5, 10), strokewidth=19, texhref="rgb24-64x64.png")
            canvas.appendChild(curve)
            return curve
        
        def setTexCoords():
            curve.texcoord1=-1
            curve.texcoord2=2
        
        canvas = self.makeEmptyCanvas()
        curve = addCurve()
        self.start(False,
                (lambda: self.compareImage("testTexturedCurve1"),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedCurve2")
                )) 

    def testPolyLine(self):
        def addPolyLine():
            polyline = avg.PolyLineNode(strokewidth=2, color="FF00FF", 
                    pos=[(10,10), (50,10), (90,50), (90, 90)])
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
            polyline2 = player.createNode(
                """<polyline strokewidth="2" color="FF00FF"
                        pos="((110,10), (100,50), (110,70))" />""")
            canvas.insertChild(polyline2,0)
        
        def testEmptyPolyLine():
            polyline2 = canvas.getChild(0)
            polyline2.pos=[]
        
        def testAlmostEmptyPolyLine():
            polyline2 = canvas.getChild(0)
            polyline2.pos=[(10,10)]
        
        def testAcutePolyLine():
            polyline2 = canvas.getChild(0)
            polyline2.strokewidth = 10
            polyline2.linejoin="bevel"
            polyline2.pos = [(50,10), (60,10), (50,11)]
            canvas.removeChild(1)
        
        canvas = self.makeEmptyCanvas()
        polyline = addPolyLine()
        self.start(False,
                (lambda: self.compareImage("testPolyLine1"),
                 changePolyLine,
                 lambda: self.compareImage("testPolyLine2"),
                 miterPolyLine,
                 lambda: self.compareImage("testPolyLine3"),
                 addPolyLine2,
                 lambda: self.compareImage("testPolyLine4"),
                 testEmptyPolyLine,
                 lambda: self.compareImage("testPolyLine5"),
                 testAlmostEmptyPolyLine,
                 lambda: self.compareImage("testPolyLine5"),
                 testAcutePolyLine,
                 lambda: self.compareImage("testPolyLine6")
                ))

    def testTexturedPolyLine(self):
        def texturePolyLine():
            polyline = avg.PolyLineNode(strokewidth=20, color="FF00FF", 
                    texhref="rgb24-64x64.png", pos=((10,10), (50,10), (90,50), (90, 90)),
                    texcoords=(0, 0.3, 0.7, 1))
            canvas.appendChild(polyline)
            return polyline
        
        def miter():
            polyline.linejoin = "miter"
        
        def setTexCoords():
            polyline.texcoords = [-1, 0, 1, 2]
        
        def repeatTexCoords():
            polyline.pos = [(10,10), (30,10), (30,50), (50,50), (50,70), (70,70)]
            polyline.texcoords = [0,1]
        
        canvas = self.makeEmptyCanvas()
        polyline = texturePolyLine()
        self.start(False,
                (lambda: self.compareImage("testTexturedPolyLine1"),
                 miter,
                 lambda: self.compareImage("testTexturedPolyLine2"),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine3"),
                 repeatTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine4")
                ))

    def testPolygon(self):
        def addPolygon():
            polygon = avg.PolygonNode(strokewidth=2, color="FF00FF",
                    pos=((10,10), (50,10), (90,50), (90, 90)))
            polygon.subscribe(avg.Node.CURSOR_DOWN, onMouseDown)
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
            pos.insert(1, (10, 10))
            pos.append((40, 40))
            polygon.pos = pos
        
        def addPolygon2():
            polygon = avg.PolygonNode(strokewidth=3, color="FF00FF",
                    pos=((100.5,10.5), (100.5,30.5), (120.5,30.5), (120.5, 10.5)))
            canvas.insertChild(polygon, 0)
        
        def miterPolygons():
            polygon.linejoin = "miter"
            polygon2 = canvas.getChild(0)
            polygon2.linejoin = "miter"
        
        def onMouseDown(event):
            self.__mouseDownCalled = True
      
        def addEmptyPolygon():
            avg.PolygonNode(parent=canvas, fillopacity=1)

        def createLeftOpenPolygon():
            polygon.pos = ( (15,0), (35,0), (55,10), (65,30), (55,50), (35,60), (15,60),
                    (5,50), (15,40), (35,40), (35,30), (35,20), (15,20), (5,10) )
            polygon.strokewidth = 2

        def createUpOpenPolygon():
            polygon.pos = ( (15,0), (25,10), (25,30), (35,30), (45,30), (45,10), (55,0), 
                    (65,10), (65,30), (55,50), (35,60), (15,50), (5,30), (5,10) )

        def createBottomOpenPolygon():
            polygon.pos = ( (35,0), (55,10), (65,30), (65,50), (55,60), (45,50), (45,30),
                    (35,30), (25,30), (25,50), (15,60), (5,50), (5,30), (15,10) )

        def createOneHole():
            polygon.holes = ( [(35,10), (40,15), (35,20), (30,15)], )

        def createMoreHoles():
            newHoles = ( polygon.holes[0], [(20,35), (20,45), (10,40)], 
                    [(50,35), (50,45), (60,40)], )
            polygon.holes = newHoles

        def clearCanvas():
            for i in xrange(canvas.getNumChildren()-1):
                dell = canvas.getChild(i)
                canvas.removeChild(dell)

        self.__mouseDownCalled = False
        canvas = self.makeEmptyCanvas()
        polygon = addPolygon()
        self.start(False,
                (lambda: self.compareImage("testPolygon1"),
                 changePolygon,
                 lambda: self.compareImage("testPolygon2"),
                 fillPolygon,
                 lambda: self.compareImage("testPolygon3"),
                 addEmptyPoint,
                 lambda: self.compareImage("testPolygon4"),
                 addPolygon2,
                 lambda: self.compareImage("testPolygon5"),
                 miterPolygons,
                 lambda: self.compareImage("testPolygon6"),
                 lambda: self.fakeClick(50, 50),
                 lambda: self.assertEqual(self.__mouseDownCalled, False),
                 lambda: self.fakeClick(20, 87),
                 lambda: self.assert_(self.__mouseDownCalled),
                 addEmptyPolygon,
                 clearCanvas,
                 createLeftOpenPolygon,
                 lambda: self.compareImage("testPolygon7"),
                 createUpOpenPolygon,
                 lambda: self.compareImage("testPolygon8"),
                 createBottomOpenPolygon,
                 lambda: self.compareImage("testPolygon9"),
                 createOneHole,
                 lambda: self.compareImage("testPolygonHole1"),
                 createMoreHoles,
                 lambda: self.compareImage("testPolygonHole2")
                ))

    def testTexturedPolygon(self):
        def texturePolygon():
            polygon = avg.PolygonNode(strokewidth=20, color="FF00FF", 
                    texhref="rgb24-64x64.png", pos=((10,10), (50,10), (90,50), (90, 90)))
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
            polygon.strokewidth=2
            polygon.filltexhref="rgb24alpha-64x64.png"
        
        def setFillTexCoords():
            polygon.filltexcoord1=(0.5, 1)
            polygon.filltexcoord2=(1.5, 3)
        
        canvas = self.makeEmptyCanvas()
        polygon = texturePolygon()
        self.start(False,
                (lambda: self.compareImage("testTexturedPolygon1"),
                 miter,
                 lambda: self.compareImage("testTexturedPolygon2"),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolygon3"),
                 repeatTexCoords,
                 lambda: self.compareImage("testTexturedPolygon4"),
                 setFillTex,
                 lambda: self.compareImage("testTexturedPolygon5"),
                 setFillTexCoords,
                 lambda: self.compareImage("testTexturedPolygon6")
                ))

    def testPointInPolygon(self):
        polygon_pos = [(10, 10), (50, 10), (90, 50), (90, 90)]
        self.assert_(avg.Point2D(50, 20).isInPolygon(polygon_pos))
        self.assert_(not avg.Point2D(10, 20).isInPolygon(polygon_pos))

    def testCircle(self):
        def addCircle():
            circle = avg.CircleNode(pos=(30, 30), r=20)
            circle.subscribe(avg.Node.CURSOR_DOWN, onMouseDown)
            canvas.appendChild(circle)
            return circle
        
        def changeCircle():
            circle.color="FF0000"
            circle.fillcolor=u"FFFFFF"
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
        
        def onMouseDown(event):
            self.__mouseDownCalled = True
        
        self.__mouseDownCalled = False
        canvas = self.makeEmptyCanvas()
        circle = addCircle()
        self.start(False,
                (lambda: self.compareImage("testCircle1"), 
                 changeCircle,
                 lambda: self.compareImage("testCircle2"),
                 textureCircle,
                 lambda: self.compareImage("testCircle3"),
                 setFillTex,
                 lambda: self.compareImage("testCircle4"),
                 setFillTexCoords,
                 lambda: self.compareImage("testCircle5"),
                 lambda: self.fakeClick(32, 32),
                 lambda: self.assert_(self.__mouseDownCalled == False),
                 lambda: self.fakeClick(67, 50),
                 lambda: self.assert_(self.__mouseDownCalled)
                ))
        
    def testMesh(self):
        def addMesh():
            div = avg.DivNode()
            mesh = avg.MeshNode(
                    texhref="rgb24-64x64.png",
                    vertexcoords=((0,0), (64,0), (0,64), (64, 64),(32, 32)),
                    texcoords=((0,0),(1,0),(0,1),(1,1),(0.5,0.5)),
                    triangles=((0,1,4),(1,3,4),(3,2,4),(2,0,4)))
            div.appendChild(mesh)
            div.x = 50
            div.y = 30
            canvas.appendChild(div)
            return mesh
        
        def setVertexCoords():
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(32, 64))
        
        def setTexCoords():
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(32, 32))
            mesh.texcoords = ((1,1),(1,1),(1,1),(1,1),(0.5,0.5))
        
        def setTriangles():
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(31.5, 32))
            mesh.texcoords = ((0,0),(1,0),(0,1),(1,1),(0.5,0.5))
            mesh.triangles = ((3,1,4),(1,3,4),(1,2,4),(2,0,4))   
            
        def setTrianglesSameItem():
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(32, 32))
            mesh.texcoords = ((0,0),(1,0),(0,1),(1,1),(0.5,0.5))
            mesh.triangles = ((1,1,1),(2,2,2),(3,3,3),(4,4,4)) 

        def setHref():
            mesh.texhref = "rgb24alpha-64x64.png"
            
        def setBackfaceCullTrue():
            mesh.texhref="rgb24-64x64.png"
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(31.5, 32))
            mesh.texcoords = ((0,0),(1,0),(0,1),(1,1),(0.5,0.5))
            mesh.triangles = ((3,1,4),(1,3,4),(1,2,4),(2,0,4))
            mesh.backfacecull = True
            
        def setBackfaceCullFalse():
            mesh.backfacecull = False 

        def setIllegalVertexes():
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(32, 32), (16,16))
           
        def setIllegalTextures():
            mesh.texcoords = ((100,0),(1,0),(0,1),(1,1),(0.5,0.5), (1.0,1.0))
            
        def setIllegalIndexes():
            mesh.triangles = ((27,1,1),(1,3,4),(3,2,4),(2,0,4)) 
        
        canvas = self.makeEmptyCanvas()
        mesh = addMesh()
        self.assertRaises(RuntimeError, setIllegalVertexes)
        self.assertRaises(RuntimeError, setIllegalTextures)
        self.assertRaises(RuntimeError, setIllegalIndexes)
        self.start(False,
                (lambda: self.compareImage("testMesh1"),
                 setVertexCoords,
                 lambda: self.compareImage("testMesh2"),
                 setTexCoords,
                 lambda: self.compareImage("testMesh3"),
                 setTriangles,
                 lambda: self.compareImage("testMesh4"),
                 setHref,
                 lambda: self.compareImage("testMesh5"),
                 setTrianglesSameItem,
                 lambda: self.compareImage("testMesh6"),
                 setBackfaceCullTrue,
                 lambda: self.compareImage("testMesh7"),
                 setBackfaceCullFalse,
                 lambda: self.compareImage("testMesh8")
                ))

    def testInactiveVector(self):
        def addVectorNode():
            node = avg.LineNode(pos1=(2, 2), pos2=(50, 2), strokewidth=2)
            canvas.appendChild(node)
            return node

        def addFilledVectorNode():
            node = avg.RectNode(pos=(2, 6), size=(50, 30), strokewidth=2)
            node.subscribe(avg.Node.CURSOR_DOWN, onDown)
            canvas.appendChild(node)
            return node

        def onDown(Event):
            vNode.active = False
            fvNode.active = False
            self.onDownCalled = not self.onDownCalled

        canvas = self.makeEmptyCanvas()
        vNode = addVectorNode()
        fvNode = addFilledVectorNode()
        self.onDownCalled = False
        self.start(False,
                (lambda: self.compareImage("testInactiveVector1"),
                 lambda: self.fakeClick(20, 20),
                 lambda: self.assert_(self.onDownCalled),
                 lambda: self.compareImage("testInactiveVector2"),
                 lambda: self.fakeClick(20, 20),
                 lambda: self.assert_(self.onDownCalled)
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
            "testPointInPolygon",
            "testCircle",
            "testMesh",
            "testInactiveVector"
            )
    return createAVGTestSuite(availableTests, VectorTestCase, tests)
