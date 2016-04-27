#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, app, sprites

class MyMainDiv(app.MainDiv):
    def onInit(self):
        self.__spritesheet = sprites.Spritesheet("spritesheet.xml")
        self.__sprite1 = sprites.AnimatedSprite(self.__spritesheet, "Ball ", loop=True,
                pos=(100,100), parent=self)
        self.__sprite1.play()
        self.__sprite2 = sprites.AnimatedSprite(self.__spritesheet, "Ball2 ", loop=True,
                pos=(150,100), parent=self)
        self.__sprite2.play()
        self.__text = avg.WordsNode(pos=(10,400), parent=self)

    def onExit(self):
        pass

    def onFrame(self):
        self.__text.text = str(int(self.__sprite1.curFrameNum))

app.App().run(MyMainDiv())

