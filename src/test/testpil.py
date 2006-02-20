#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, syslog
# TODO: set this path via configure or something similar.
sys.path.append('/usr/local/lib/python2.3/site-packages/libavg')
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
import avg

class PILTestCase(unittest.TestCase):
    def test(self):
        def getImage():
            node = Player.getElementByID("test")
            print "(", node.imagesize.x, ", ", node.imagesize.y, "), ", node.imageformat
            print len(node.getImage())
#            print node.getImage()
        Player.loadFile("image.avg")
        Player.setTimeout(100, getImage)
        Player.setTimeout(250, Player.stop)
        Player.play()
        self.assert_(Player.isPlaying() == 0)

Player = avg.Player()
runner = unittest.TextTestRunner()
runner.run(PILTestCase("test"))

