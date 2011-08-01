# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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

import unittest

from libavg import avg
from testcase import *

class VectorTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)

    def makeEmptyCanvas(self):
        self.loadEmptyScene()
        canvas = avg.DivNode(id="canvas", size=(160,120), parent=Player.getRootNode())
        return canvas

    def testLine(self):
        def addLines():
            def addLine(attribs):
                line = Player.createNode("line", attribs)
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
        self.start((
                 lambda: self.compareImage("testline1", False), 
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
                line = avg.LineNode(pos1=(2, y), pos2=(10, y))
                canvas.appendChild(line)
       
        canvas = self.makeEmptyCanvas()
        self.start((
                 addLines,
                 lambda: self.compareImage("testlotsoflines", False), 
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
            bmp = avg.Bitmap("rgb24alpha-64x64.png")
            self.line.setBitmap(bmp)
        
        def bmpNoTexture():
            self.line.setBitmap(None)
        
        canvas = self.makeEmptyCanvas()
        addLine()
        self.start((
                 lambda: self.compareImage("testtexturedline1", False), 
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
                 lambda: self.compareImage("testtexturedline4", False),
                 bmpNoTexture,
                 lambda: self.compareImage("testtexturedline5", False),
                ))

    def testLineOpacity(self):
        def addLine():
            line = avg.LineNode(pos1=(2, 2.5), pos2=(158, 2.5), opacity=0.5)
            canvas.appendChild(line)
        
        def changeCanvasOpacity():
            canvas.opacity = 0.5
            canvas.getChild(0).opacity = 0.25
        
        canvas = self.makeEmptyCanvas()
        self.start((
                 addLine,
                 lambda: self.compareImage("testlineopacity1", False), 
                 changeCanvasOpacity,
                 lambda: self.compareImage("testlineopacity2", False), 
                ))

    def testRect(self):
        def addRect():
            rect = avg.RectNode(pos=(2, 2), size=(50, 30), fillopacity=1, 
                    strokewidth=0)
            canvas.appendChild(rect)
            rect.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown)
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
        helper = Player.getTestHelper()
        self.start((
                 lambda: self.compareImage("testRect1", False),
                 moveRect,
                 lambda: self.compareImage("testRect2", False),
                 rotateRect,
                 lambda: self.compareImage("testRect3", False),
                 addRect2,
                 lambda: self.compareImage("testRect4", False),
                 lambda: self.fakeClick(100, 100),
                 lambda: self.assert_(self.__mouseDownCalled == False),
                 lambda: self.fakeClick(55, 50),
                 lambda: self.assert_(self.__mouseDownCalled == False),
                 lambda: self.fakeClick(65, 60),
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
            self.rect = Player.createNode(
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
            bmp = avg.Bitmap("rgb24-64x64.png")
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
        self.start((
                 lambda: self.compareImage("testTexturedRect1", False),
                 newRect,
                 lambda: self.compareImage("testTexturedRect2", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedRect3", False),
                 setFillTex,
                 lambda: self.compareImage("testTexturedRect4", False),
                 setFillTexCoords,
                 lambda: self.compareImage("testTexturedRect5", False),
                 setFillBitmap,
                 lambda: self.compareImage("testTexturedRect6", False),
                 clearFillBitmap,
                 lambda: self.compareImage("testTexturedRect7", False),
                 setFillBitmap,
                 lambda: self.compareImage("testTexturedRect6", False),
#                 setTransparentBorder,
#                 lambda: self.compareImage("testTexturedRect8", False),
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
        self.start((
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
            curve = avg.CurveNode(pos1=(10.5, 10), pos2=(10.5, 80), pos3=(80.5, 80), 
                 pos4=(80.5, 10), strokewidth=19, texhref="rgb24-64x64.png")
            canvas.appendChild(curve)
            return curve
        
        def setTexCoords():
            curve.texcoord1=-1
            curve.texcoord2=2
        
        canvas = self.makeEmptyCanvas()
        curve = addCurve()
        self.start((
                 lambda: self.compareImage("testTexturedCurve1", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedCurve2", False)
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
            polyline2 = Player.createNode(
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
        self.start((
                 lambda: self.compareImage("testPolyLine1", False),
                 changePolyLine,
                 lambda: self.compareImage("testPolyLine2", False),
                 miterPolyLine,
                 lambda: self.compareImage("testPolyLine3", False),
                 addPolyLine2,
                 lambda: self.compareImage("testPolyLine4", False),
                 testEmptyPolyLine,
                 lambda: self.compareImage("testPolyLine5", False),
                 testAlmostEmptyPolyLine,
                 lambda: self.compareImage("testPolyLine5", False),
                 testAcutePolyLine,
                 lambda: self.compareImage("testPolyLine6", False)
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
            polyline.texcoords = [1, 2, 3]
        
        canvas = self.makeEmptyCanvas()
        polyline = texturePolyLine()
        self.start((
                 lambda: self.compareImage("testTexturedPolyLine1", False),
                 miter,
                 lambda: self.compareImage("testTexturedPolyLine2", False),
                 setTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine3", False),
                 repeatTexCoords,
                 lambda: self.compareImage("testTexturedPolyLine4", False)
                ))

    def testPolygon(self):
        def addPolygon():
            polygon = avg.PolygonNode(strokewidth=2, color="FF00FF",
                    pos=((10,10), (50,10), (90,50), (90, 90)))
            polygon.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown)
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

        self.__mouseDownCalled = False
        canvas = self.makeEmptyCanvas()
        polygon = addPolygon()
        helper = Player.getTestHelper()
        self.start((
                 lambda: self.compareImage("testPolygon1", True),
                 changePolygon,
                 lambda: self.compareImage("testPolygon2", True),
                 fillPolygon,
                 lambda: self.compareImage("testPolygon3", True),
                 addEmptyPoint,
                 lambda: self.compareImage("testPolygon4", True),
                 addPolygon2,
                 lambda: self.compareImage("testPolygon5", True),
                 miterPolygons,
                 lambda: self.compareImage("testPolygon6", False),
                 lambda: self.fakeClick(50, 50),
                 lambda: self.assert_(self.__mouseDownCalled == False),
                 lambda: self.fakeClick(20, 87),
                 lambda: self.assert_(self.__mouseDownCalled),
                 addEmptyPolygon
                ))

    def testSelfIntersectPolygon(self):
        def addPolygon():
            polygon = avg.PolygonNode(strokewidth=3, color="FF00FF",
                    pos=((100.5, 10.5), (100.5, 30.5), (120.5, 10.5), (120.5, 30.5)),
                    fillcolor="00FFFF", fillopacity=0.5)
            canvas.insertChild(polygon, 0)
                
        canvas = self.makeEmptyCanvas()
        self.assertException(lambda: self.start([addPolygon]))

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
        self.start((
                 lambda: self.compareImage("testTexturedPolygon1", False),
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

    def testPointInPolygon(self):
        polygon_pos = [(10, 10), (50, 10), (90, 50), (90, 90)]
        self.assert_(avg.pointInPolygon((50, 20), polygon_pos))
        self.assert_(avg.pointInPolygon((10, 20), polygon_pos) == False)

    def testCircle(self):
        def addCircle():
            circle = avg.CircleNode(pos=(30, 30), r=20)
            circle.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onMouseDown)
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
        
        def onMouseDown(event):
            self.__mouseDownCalled = True
        
        self.__mouseDownCalled = False
        canvas = self.makeEmptyCanvas()
        circle = addCircle()
        helper = Player.getTestHelper()
        self.start((
                 lambda: self.compareImage("testCircle1", False), 
                 changeCircle,
                 lambda: self.compareImage("testCircle2", False),
                 textureCircle,
                 lambda: self.compareImage("testCircle3", False),
                 setFillTex,
                 lambda: self.compareImage("testCircle4", False),
                 setFillTexCoords,
                 lambda: self.compareImage("testCircle5", False),
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

        def setIllegalVertexes():
            mesh.vertexcoords = ((0,0), (64,0), (0,64), (64, 64),(32, 32), (16,16))
           
        def setIllegalTextures():
            mesh.texcoords = ((100,0),(1,0),(0,1),(1,1),(0.5,0.5), (1.0,1.0))
            
        def setIllegalIndexes():
            mesh.triangles = ((27,1,1),(1,3,4),(3,2,4),(2,0,4)) 
        
        canvas = self.makeEmptyCanvas()
        mesh = addMesh()
        self.assertException(setIllegalVertexes)
        self.assertException(setIllegalTextures)
        self.assertException(setIllegalIndexes)
        self.start((
                 lambda: self.compareImage("testMesh1", False),
                 setVertexCoords,
                 lambda: self.compareImage("testMesh2", False),
                 setTexCoords,
                 lambda: self.compareImage("testMesh3", False),
                 setTriangles,
                 lambda: self.compareImage("testMesh4", False),
                 setHref,
                 lambda: self.compareImage("testMesh5", False),
                 setTrianglesSameItem,
                 lambda: self.compareImage("testMesh6", False)
                ))

    def testInactiveVector(self):
        def addVectorNode():
            node = avg.LineNode(pos1=(2, 2), pos2=(50, 2), strokewidth=2)
            canvas.appendChild(node)
            return node

        def addFilledVectorNode():
            node = avg.RectNode(pos=(2, 6), size=(50, 30), strokewidth=2)
            node.setEventHandler(avg.CURSORDOWN, avg.MOUSE, onDown)
            canvas.appendChild(node)
            return node

        def onDown(Event):
            vNode.active = False
            fvNode.active = False
            self.onDownCalled = not self.onDownCalled

        Helper = Player.getTestHelper()
        canvas = self.makeEmptyCanvas()
        vNode = addVectorNode()
        fvNode = addFilledVectorNode()
        self.onDownCalled = False
        self.start((
                 lambda: self.compareImage("testInactiveVector1", False),
                 lambda: self.fakeClick(20, 20),
                 lambda: self.assert_(self.onDownCalled),
                 lambda: self.compareImage("testInactiveVector2", False),
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
            "testSelfIntersectPolygon",
            "testTexturedPolygon",
            "testPointInPolygon",
            "testCircle",
            "testMesh",
            "testInactiveVector"
            )
    return createAVGTestSuite(availableTests, VectorTestCase, tests)

Player = avg.Player.get()
