#!/usr/bin/env python
# -*- coding: utf-8 -*-

import libavg
from libavg import app


class MyMainDiv(app.MainDiv):
    # An OptionParser instance is passed to this function, allowing the MainDiv to
    # add command line arguments
    def onArgvParserCreated(self, parser):
        parser.add_option('--speed', '-s', default='300', dest='speed',
                help='Pixels per second')
        parser.add_option('--color', '-c', default='ff0000', dest='color',
                help='Fill color of the running block')

    # This method is called when the command line options are being parsed.
    # options, args are the result of OptionParser.parse_args().
    def onArgvParsed(self, options, args, parser):
        self.argvoptions = options

    # This is called as soon as the player is started by the App object.
    # Initialize everything here.
    def onInit(self):
        libavg.WordsNode(text='block_speed=%s app_resolution=%s' %
                (self.argvoptions.speed,
                self.settings.getPoint2D('app_resolution')),
                pos=(10, self.height - 25), parent=self)

        # Create a graphic element that will be animated
        self.__runningBlock = libavg.RectNode(pos=(0, 100), size=(20, 20),
                fillopacity=1, fillcolor=self.argvoptions.color,
                parent=self)
        self.__shouldMove = True

        app.keyboardmanager.bindKeyDown('m', self.__toggleMotion, 'Toggle motion')

    def onExit(self):
        print 'Exiting..'

    def onFrame(self, delta):
        # delta is the time, in seconds, since the last call to this function
        if self.__shouldMove:
            speed = float(self.argvoptions.speed)
            self.__runningBlock.pos += (speed * delta, 0)
            if self.__runningBlock.pos.x > self.size.x:
                self.__runningBlock.pos = (0, 100)

    def __toggleMotion(self):
        self.__shouldMove = not self.__shouldMove
        # Flash messages are debug notifications that are shown temporarily on top of all
        # the visible elements.
        app.flashmessage.FlashMessage('Should move: %s' % self.__shouldMove)


if __name__ == '__main__':
    # App options (such as app_resolution) can be changed as parameters of App().run()
    app.App().run(MyMainDiv(), app_resolution='1024x500')

