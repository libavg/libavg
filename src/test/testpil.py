#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, syslog
# TODO: set this path via configure or something similar.
sys.path.append('/usr/local/lib/python2.3/site-packages/libavg')
sys.path.append('/usr/local/lib/python2.4/site-packages/libavg')
sys.path.append('/usr/local/lib/python2.4/site-packages/PIL')
import avg

import Image, ImageFilter


class PILTestCase(unittest.TestCase):
    def test(self):
        def getImage():
            node = Player.getElementByID("test")
            bitmap = node.getBitmap()
            print bitmap.getSize(), bitmap.getFormat()
            print len(bitmap.getPixels())
            im = Image.fromstring("RGB", bitmap.getSize(), bitmap.getPixels())
            im1 = im.filter(ImageFilter.SMOOTH_MORE)
            bitmap.setPixels(im1.tostring())
            node.setBitmap(bitmap)
        Player.loadFile("image.avg")
        Player.setTimeout(100, getImage)
#        Player.setTimeout(250, Player.stop)
        Player.play()
        self.assert_(Player.isPlaying() == 0)

Player = avg.Player()
runner = unittest.TextTestRunner()
runner.run(PILTestCase("test"))

