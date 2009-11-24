#!/usr/bin/env python
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

import sys, time, platform, os.path, shutil
import math

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

# features to test:
#   href
#   pos
#   size
#   warp, tile size
#   getBitmap
#   blendmode
#   maskhref, maskpos, masksize
#   mipmap
# 
# situations to test features in:
#   xml ctor before play
#   xml ctor after play
#   dict ctor before play
#   dict ctor after play
#   before play, before attach
#   before play, after attach
#   after play, before attach
#   after play, after attach


class ImageTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def testHRef(self):
        def createXmlNode(pos):
            return Player.createNode(
                    """<image pos="%s" href="rgb24-32x32.png"/>"""%str(pos))        

        def createDictNode(pos):
            return Player.createNode("image", {"pos":pos, "href":"rgb24-32x32.png"})       
        def addNodes(y):
            root = Player.getRootNode()
            xmlNode = createXmlNode((16, y))
            root.appendChild(xmlNode)
            dictNode = createDictNode((48, y))
            root.appendChild(dictNode)
            noAttachNode = createXmlNode((80, y))
            noAttachNode.href = "rgb24alpha-32x32.png"
            root.appendChild(noAttachNode)
            attachNode = createXmlNode((112, y))
            root.appendChild(attachNode)
            attachNode.href = "rgb24alpha-32x32.png"

        self._loadEmpty()
        addNodes(16)
        self.start(None,
                (lambda: self.compareImage("testImgHRef1", False),
                 lambda: addNodes(48),
                 lambda: self.compareImage("testImgHRef2", False),
                ))
        

def imageTestSuite(tests):
    availableTests = ("testHRef",
            )
    return AVGTestSuite(availableTests, ImageTestCase, tests)

Player = avg.Player.get()

