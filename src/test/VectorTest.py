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

    def testLine(self):
        def addLines():
            def addLine(attribs):
                line = Player.createNode("line", attribs)
                canvas.appendChild(line)
            canvas = Player.getElementByID("canvas")
            addLine({"x1":2, "y1":2.5, "x2":10, "y2":2.5})
            addLine({"x1":11.5, "y1":4, "x2":11.5, "y2":12})
            addLine({"x1":14, "y1":3, "x2":22, "y2":3, "width":2})
            addLine({"x1":24, "y1":5, "x2":24, "y2":13, "width":2})
            addLine({"x1":2, "y1":13.5, "x2":10, "y2":13.5, "color":"FF0000"})
            addLine({"x1":2, "y1":15.5, "x2":10, "y2":15.5, "color":"00FF00"})
            addLine({"x1":2, "y1":17.5, "x2":10, "y2":17.5, "color":"0000FF"})
        self.makeEmptyCanvas()
        self.start(None,
                (addLines,
                 lambda: self.compareImage("testline", False), 
                ))

    def testLotsOfLines(self):
        def addLines():
            canvas = Player.getElementByID("canvas")
            for i in xrange(5000):
                y = i/10+2.5
                line = Player.createNode("line", {"x1":2, "y1":y, "x2":10, "y2":y})
                canvas.appendChild(line)
                
        self.makeEmptyCanvas()
        self.start(None,
                (addLines,
                 lambda: self.compareImage("testlotsoflines", False), 
                ))
        


def vectorTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(VectorTestCase("testLine"))
    suite.addTest(VectorTestCase("testLotsOfLines"))
    return suite

Player = avg.Player.get()
