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
    # Add options that your code needs
    EXTRA_OPTIONS = [Option('foo_bar', 'baz', 'Help text for foo_bar option')]

    def onInit(self):
        libavg.WordsNode(text='foo_bar=%s app_resolution=%s' %
                (app.instance.settings.get('foo_bar'),
                app.instance.settings.getpoint2d('app_resolution')), parent=self)

# parameters passed as kwargs to the App.run method will override the wired defaults,
# but they're overriden by the command line arguments
app.App().run(MyScene(), foo_bar='baz2')

