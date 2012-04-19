#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *
from libavg import ui, utils

g_Player = avg.Player.get()

class Main(AVGApp):
    multitouch = True
    
    def __init__(self, parentNode):
        parentNode.mediadir = utils.getMediaDir("../test/media")
        self.rootNode = avg.DivNode(parent = parentNode)
        super(Main, self).__init__(parentNode)
    
    def init(self):
        self.button = ui.TouchButton.fromSrc("button_up.png", "button_down.png",
                "button_disabled.png", parent=self.rootNode,
                clickHandler=self.clickHandler)
        
        self.toggleButton = ui.ToggleButton.fromSrc("toggle_unchecked_Up.png",
                "toggle_unchecked_Down.png", "toggle_checked_Up.png",
                "toggle_checked_Down.png", "toggle_unchecked_Disabled.png",
                "toggle_checked_Disabled.png", checkHandler=self.checkedHandler,
                uncheckHandler=self.uncheckedHandler, parent=self.rootNode, pos=(100, 10))
        
    def onKeyDown(self, event):
        if event.keystring == "left":
            self.button.setEnabled(not self.button.getEnabled())
            self.toggleButton.setEnabled(not self.toggleButton.getEnabled())
        if event.keystring == "right":
            print "State:",self.toggleButton.getState()
            print "Checked:",self.toggleButton.getChecked()
            print "Enabled:",self.toggleButton.getEnabled(),"\n"
        if event.keystring == "#":
            self.toggleButton.setChecked(not self.toggleButton.getChecked())
    
    def clickHandler(self):
        print "CLICKED"
    
    def checkedHandler(self, event):
        print "CHECKED"
    
    def uncheckedHandler(self, event):
        print "UNCHECKED"
        
if __name__ == '__main__':
    Main.start()
