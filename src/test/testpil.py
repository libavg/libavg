#!/usr/bin/python
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

Player = avg.Player.get()
runner = unittest.TextTestRunner()
runner.run(PILTestCase("test"))

