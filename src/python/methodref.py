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

import weakref, new

class methodref(object):
    # From Python Cookbook
    """ Wraps any callable, most importantly a bound method, in a way that allows a bound
        method's object to be GC'ed, while providing the same interface as a normal weak
        reference."""
    def __init__(self, fn):
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
                self.__name__ = None
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

    def isSameFunc(self, func):
        if self._obj is None:
            return func == self._func
        elif self._obj() is None:
            return func is None
        else:
            try:
                o, f, c = func.im_self, func.im_func, func.im_class
            except AttributeError:
                return False
            else:
                return (o == self._obj() and f == self._func and c == self._clas)

    def __call__(self):
        if self._obj is None:
            return self._func
        elif self._obj() is None:
            return None
        return new.instancemethod(self._func, self._obj(), self._clas)
