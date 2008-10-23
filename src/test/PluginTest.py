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

class PluginTestCase(unittest.TestCase):
    def runTest(self):
        self.Log = avg.Logger.get()
        self.Log.setCategories(self.Log.APP |
                self.Log.WARNING |
                self.Log.PLUGIN

#                  self.Log.PROFILE |
#                  self.Log.PROFILE_LATEFRAMES |
#                  self.Log.CONFIG |
#                  self.Log.MEMORY |
#                  self.Log.BLTS    |
#                  self.Log.EVENTS |
#                  self.Log.EVENTS2
                  )
                
        player = avg.Player.get()
        player.pluginPath = "../player/testplugin"
        player.loadPlugin("ColorNode")

        Player.loadString("""
          <avg width="160" height="120">
              <div width="160" height="120">
                 <colornode id="mynode" fillcolor="7f7f00" />
              </div>
          </avg>
        """)
        mynode = Player.getElementByID("mynode")
        self.assert_(mynode.fillcolor == "7f7f00")

def pluginTestSuite (tests):
    suite = unittest.TestSuite()
    suite.addTest (PluginTestCase())
    return suite

Player = avg.Player.get()

