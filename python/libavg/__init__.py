#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2021 Ulrich von Zadow
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

# Work around libstdc++ Mesa bug
# (https://bugs.launchpad.net/ubuntu/+source/mesa/+bug/259219)
from platform import system
if system() == 'Linux':
    from ctypes import cdll
    cdll.LoadLibrary("libpixman-1.so.0")
    cdll.LoadLibrary("libstdc++.so.6")
del system

from avg import *
player = avg.Player.get()

import textarea
import statemachine
import utils, methodref
import gesture
import filter
import persist
import app

