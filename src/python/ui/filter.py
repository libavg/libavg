# -*- coding: utf-8 -*-
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

import math

# Input filter based on:
# Casiez, G., Roussel, N. and Vogel, D. (2012). 1€ Filter: A Simple Speed-based Low-pass
# Filter for Noisy Input in Interactive Systems. Proceedings of the ACM Conference on
# Human Factors in Computing Systems (CHI '12). Austin, Texas (May 5-12, 2012). New York:
# ACM Press, pp. 2527-2530.

class LowPassFilter(object):

    def __init__(self, alpha):
        self.__setAlpha(alpha)
        self.__y = None
        self.__s = None

    def __setAlpha(self, alpha):
        alpha = float(alpha)
        if alpha <= 0 or alpha > 1.0:
            raise RuntimeError("LowPassFilter alpha (%s) should be in (0.0, 1.0]"%alpha)
        self.__alpha = alpha

    def apply(self, value, timestamp=None, alpha=None):        
        if alpha is not None:
            self.__setAlpha(alpha)
        if self.__y is None:
            s = value
        else:
            s = self.__alpha*value + (1.0-self.__alpha)*self.__s
        self.__y = value
        self.__s = s
        return s

    def lastValue(self):
        return self.__y


class OneEuroFilter(object):

    def __init__(self, mincutoff=1.0, beta=0.0, dcutoff=1.0):
        if mincutoff<=0:
            raise ValueError("mincutoff should be >0")
        if dcutoff<=0:
            raise ValueError("dcutoff should be >0")
        self.__freq = 60     # Initial freq, updated as soon as we have > 1 sample
        self.__mincutoff = float(mincutoff)
        self.__beta = float(beta)
        self.__dcutoff = float(dcutoff)
        self.__x = LowPassFilter(self.__alpha(self.__mincutoff))
        self.__dx = LowPassFilter(self.__alpha(self.__dcutoff))
        self.__lasttime = None
        
    def __alpha(self, cutoff):
        te    = 1.0 / self.__freq
        tau   = 1.0 / (2*math.pi*cutoff)
        return  1.0 / (1.0 + tau/te)

    def apply(self, x, timestamp):
        timestamp /= 1000.
        if self.__lasttime == timestamp:
            return x
        else:
            # ---- update the sampling frequency based on timestamps
            if self.__lasttime and timestamp:
                self.__freq = 1.0 / (timestamp-self.__lasttime)
            self.__lasttime = timestamp
            # ---- estimate the current variation per second
            prev_x = self.__x.lastValue()
            dx = 0.0 if prev_x is None else (x-prev_x)*self.__freq # FIXME: 0.0 or value?
            edx = self.__dx.apply(dx, timestamp, alpha=self.__alpha(self.__dcutoff))
            # ---- use it to update the cutoff frequency
            cutoff = self.__mincutoff + self.__beta*math.fabs(edx)
            # ---- filter the given value
            return self.__x.apply(x, timestamp, alpha=self.__alpha(cutoff))

