#!/usr/bin/python

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg

Log = avg.Logger.get()
Log.setCategories(Log.APP)

Player = avg.Player()
Player.loadFile("image.avg");
Player.play(30);
