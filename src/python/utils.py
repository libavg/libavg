# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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

import weakref, new

from libavg import avg, mathutil

g_Player = avg.Player.get()

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
    node =  g_Player.createNode('image', {'href': absHref})
    node.size = mathutil.getScaledDim(node.size, max = maxSize)
    return node

def initFXCache(numFXNodes):
    nodes = []
    mediadir = os.path.join(os.path.dirname(__file__), 'data')
    for i in range(numFXNodes):
        node = avg.ImageNode(href=mediadir+"/black.png", 
                parent=g_Player.getRootNode())
        node.setEffect(avg.NullFXNode())
        nodes.append(node)
    for node in nodes:
        node.unlink(True)

class methodref(object):
    # From Python Cookbook
    """ Wraps any callable, most importantly a bound method, in a way that allows a bound
        method's object to be GC'ed, while providing the same interface as a normal weak
        reference."""
    def __init__(self, fn):
        self.__name__ = None
        try:
            # Try getting object, function and class
            o, f, c = fn.im_self, fn.im_func, fn.im_class
        except AttributeError:
            # It's not a bound method
            self._obj = None
            self._func = fn
            self._clas = None
            if fn:
                self.__name__ =  fn.__name__
        else:
            # Bound method
            if o is None:        # ... actually UN-bound
                self._obj = None
                self.__name__ =  f.__name__
            else:
                self._obj = weakref.ref(o)
                self.__name__ =  fn.im_class.__name__ + "." + fn.__name__
            self._func = f
            self._clas = c

    def __call__(self):
        if self._obj is None:
            return self._func
        elif self._obj() is None:
            return None
        return new.instancemethod(self._func, self._obj(), self._clas)

def callWeakRef(ref, *args, **kwargs):
    func = ref()
    if func is None:
        return
    else:
        return func(*args, **kwargs)

