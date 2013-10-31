'''
libavg is a high-level development platform for media-centric applications.
https://www.libavg.de
'''

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

from enumcompat import *

import textarea
import statemachine
from avgapp import AVGApp
from appstarter import AVGAppStarter, AVGMTAppStarter, AppStarter
import utils, methodref
import gesture
import filter
import persist
import app

