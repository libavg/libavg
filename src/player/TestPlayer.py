#!/usr/bin/python

import sys
sys.path.append('/usr/local/lib/python2.3/site-packages/avg')
import avg

Log = avg.Logger.get()
Log.setCategories(Log.APP)

Image = avg.Image()
print(Image)
print(Image.ID)
print(Image.href)
Image.x = 10
print(Image.x)
Player = avg.Player()

Player.loadFile("test/image.avg")
Player.play(30)

Player.loadFile("test/hugeimage.avg")
Player.play(30)

Player.loadFile("test/noxml.avg")
#Player.loadFile("test/noavg.avg")

Player.loadFile("test/crop2.avg")
Player.play(30)

Player.loadFile("test/excl.avg")
Player.play(30)

Player.loadFile("test/video.avg")
Player.play(30)

