# Work around libstdc++ Mesa bug
# (https://bugs.launchpad.net/ubuntu/+source/mesa/+bug/259219)
from platform import system
if system() == 'Linux':
    from ctypes import cdll
    cdll.LoadLibrary("libstdc++.so.6")

from avg import *
import anim
import draggable
import camcalibrator
import textarea
from grabbable import Grabbable
from AVGApp import AVGApp
from AVGAppStarter import AVGAppStarter
from AVGMTAppStarter import AVGMTAppStarter
import AVGAppUtil
import gameapp
