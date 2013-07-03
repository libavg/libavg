#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
More complete application example employing settings and common MainDiv placeholders.

Try also:
$ ./app_settings.py -h
$ ./app_settings.py --block-speed=900
$ ./app_settings.py --block-speed 750 --block-color 00ff00

Note: animations are usually more efficiently handled by libavg.avg.Anim classes.
'''

import libavg
import libavg.widget
from libavg import app
from libavg.app.settings import Option


class MyMainDiv(app.MainDiv):
    def onArgvExtenderCreated(self, argvExtender):
        argvExtender.parser.add_option('--speed', '-s', default='300', dest='speed',
                help='Pixels per second')
        argvExtender.parser.add_option('--color', '-c', default='ff0000', dest='color',
                help='Fill color of the running block')

    def onArgvExtenderApplied(self, argvExtender):
        self.argvoptions = argvExtender.parsedArgs[0]

    # This is called as soon as the player is started by the App()
    # it's supposed to be the place where everything gets initialized.
    def onInit(self):
        libavg.WordsNode(text='block_speed=%s app_resolution=%s' %
                (self.argvoptions.speed,
                app.instance.settings.getpoint2d('app_resolution')),
                pos=(10, self.height - 25), parent=self)

        # Create a graphic element that will be animated
        self.__runningBlock = libavg.RectNode(pos=(0, 100), size=(20, 20),
                fillopacity=1, fillcolor=self.argvoptions.color,
                parent=self)
        self.__shouldMove = True

        # Add a button to switch off/on the motion of the running block
        button = libavg.widget.TextButton(text='Toggle motion', pos=(20, 200),
                size=(100, 30), parent=self)
        button.subscribe(button.CLICKED, self.__toggleMotion)

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
        # Flash messages are notifications that are shown temporarily on top of all
        # the visible elements.
        app.flashmessage.FlashMessage('Should move: %s' % self.__shouldMove)


if __name__ == '__main__':
    app.App().run(MyMainDiv())

