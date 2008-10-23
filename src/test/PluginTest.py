#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

import sys, os, platform

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess. 
sys.path += ['../wrapper/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg    # Under windows, there is no uninstalled version.
else:    
    import avg

from testcase import *

class PluginTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def testColorNodePlugin(self):
        def loadPlugin():
            if platform.system() == 'Windows':
                Player.pluginPath = "../../../../Archimedes/libavg_win/debug"
            else:
                Player.pluginPath = "../player/testplugin"
            Player.loadPlugin("ColorNode")

        def usePlugin():
            Player.stop()
            Player.loadString("""
            <avg width="160" height="120">
                <div width="160" height="120">
                    <colornode id="mynode" fillcolor="7f7f00" />
                </div>
            </avg>""")
            mynode = Player.getElementByID("mynode")
            self.assert_(mynode.fillcolor == "7f7f00")
            Player.play()
        
        self._loadEmpty()
        self.start(None, (
            loadPlugin,
            lambda: True, 
            usePlugin,
            lambda: True #self.compareImage("testline2", False),
        ))

def pluginTestSuite (tests):
    availableTests = (
        "testColorNodePlugin",
        )    
    return AVGTestSuite (availableTests, PluginTestCase, tests)
    
Player = avg.Player.get()
Log = avg.Logger.get()
Log.setCategories(
    Log.APP |
    Log.WARNING |
    Log.PLUGIN
)

if __name__ == '__main__':
    runStandaloneTest (pluginTestSuite)