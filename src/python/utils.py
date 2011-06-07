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
# Original author of this file is Martin Heistermann <mh at sponc dot de>
#

import os
from libavg import avg, mathutil

g_player = avg.Player.get()

def getMediaDir(_file_, subdir='media'):
    """call with _file_=__file__"""
    myDir = os.path.dirname(_file_)
    mediaDir = os.path.join(myDir, subdir)
    return os.path.abspath(mediaDir)

def getMediaDirFromNode(node, path=''):
    '''
    Recursively build the mediadir path, starting from the given node.
    '''
    if node.getParent():
        if type(node) in (avg.DivNode, avg.AVGNode):
            return getMediaDirFromNode(node.getParent(), os.path.join(node.mediadir, path))
        else:
            return getMediaDirFromNode(node.getParent(), path)
    else:
        return path

def createImagePreviewNode(maxSize, absHref):
    node =  g_player.createNode('image', {'href': absHref})
    node.size = mathutil.getScaledDim(node.size, max = maxSize)
    return node

