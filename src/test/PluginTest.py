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
    sys.path += ['../..']	  # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg	  # Under windows, there is no uninstalled version.
else:	 
    import avg

from testcase import *

class PluginTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)

    def testColorNodePlugin(self):
        def loadPlugin():
            if platform.system() == 'Windows':
                Player.loadPlugin("ColorNode")
            else:
                Player.pluginPath = "./plugin/.libs"
                Player.loadPlugin("libColorNode")
            
        def usePlugin1():
            node = Player.createNode("colornode", {"fillcolor":"7f7f00", "id":"mynode1"})
            Player.getElementByID("container").appendChild(node)
            
            mynode = Player.getElementByID("mynode1")
            self.assert_(mynode.fillcolor == "7f7f00")
 
        def usePlugin2():
            node = Player.createNode('<colornode fillcolor="0f3f7f" id="mynode2" />')
            Player.getElementByID("container").appendChild(node)

            mynode = Player.getElementByID("mynode2")
            self.assert_(mynode.fillcolor == "0f3f7f")

       
        Player.loadString("""
            <avg width="160" height="120" id="container" />""")

        self.start(None, (
            loadPlugin,
            usePlugin1,
            lambda: self.compareImage("testplugin1", False),
            usePlugin2,
            lambda: self.compareImage("testplugin2", False),
        ))

def pluginTestSuite (tests):
    availableTests = ("testColorNodePlugin",)
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
