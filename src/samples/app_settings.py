#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
How to use the settings backend to store and retrieve a keyed value.
Try also:
$ ./app_settings.py -h
$ ./app_settings.py --foo-bar=whatever
'''

import libavg
from libavg import app
from libavg.app.settings import Option

class MyScene(app.MainDiv):
    def onInit(self):
        libavg.WordsNode(text='foo_bar=%s app_resolution=%s' %
                (app.instance.settings.get('foo_bar'),
                app.instance.settings.getpoint2d('app_resolution')), parent=self)

myApp = app.App()
myApp.settings.addOption(Option('foo_bar', 'baz', 'Help text for foo_bar option'))

# parameters passed as kwargs to the App.run method will override the wired defaults,
# but they're overriden by the command line arguments
myApp.run(MyScene(), foo_bar='baz2')

