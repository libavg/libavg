#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import avg, gesture, app, player

RESOLUTION = avg.Point2D(800, 600)

class TextRect(avg.DivNode):
    def __init__(self, text, parent=None, **kwargs):
        super(TextRect, self).__init__(size=(150,40), **kwargs)
        if parent:
            parent.appendChild(self)

        self.rect = avg.RectNode(size=self.size, fillopacity=1, fillcolor="000000", 
                color="FFFFFF", parent=self)
        self.words = avg.WordsNode(color="FFFFFF", text=text, alignment="center", 
                parent=self)
        self.words.pos = (self.size-(0,self.words.size.y)) / 2
        self.name = text

div = avg.DivNode()
Huey  = TextRect("Huey"); Huey.y  = 00
Dewey = TextRect("Dewey"); Dewey.y = 40
Louie = TextRect("Louie"); Louie.y = 80

div.appendChild(Huey )
div.appendChild(Dewey)
div.appendChild(Louie)


for child in div.children:
    print child.name


canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()

rootNode.appendChild(div)

player.play()
