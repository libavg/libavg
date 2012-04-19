#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libavg import *
from libavg import ui, utils

g_Player = avg.Player.get()

class Main(AVGApp):
    multitouch = True
    
    def __init__(self, parentNode):
        parentNode.mediadir = utils.getMediaDir("../test/media")#__file__)
        self.rootNode = avg.DivNode(parent = parentNode)
        super(Main, self).__init__(parentNode)
    
    def init(self):
        self.button = ui.TouchButton(upNode=avg.ImageNode(href="button_up.png"),
                downNode=avg.ImageNode(href="button_down.png"),
                disabledNode=avg.ImageNode(href="button_disabled.png"),
                parent=self.rootNode, clickHandler=self.clickHandler
                )
        
        self.toggleButton = ui.ToggleButton(
                uncheckedUpNode=avg.ImageNode(href="toggle_unchecked_Up.png"),
                uncheckedDownNode=avg.ImageNode(href="toggle_unchecked_Down.png"),
                checkedUpNode=avg.ImageNode(href="toggle_checked_Up.png"),
                checkedDownNode=avg.ImageNode(href="toggle_checked_Down.png"),
                uncheckedDisabledNode=avg.ImageNode(href="toggle_unchecked_Disabled.png"),
                checkedDisabledNode=avg.ImageNode(href="toggle_checked_Disabled.png"),
                checkHandler=self.checkedHandler, uncheckHandler=self.uncheckedHandler,
                parent=self.rootNode, pos=(100, 0)
                )
        
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
