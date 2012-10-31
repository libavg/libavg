#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Shows how to use BitmapManager to asynchronously load a Bitmap from a file.
Run this snippet providing a list of filenames of (high resolution) pictures:

$ ./asyncload.py /path/to/mypics/*.jpg anotherpic.png nonexistent.png

Press space to sequentially load the pictures. A rotating rectangle appears
during the time the picture file is being loaded to show how the main thread
is not affected by the load operation.

Press 'f' to display the frame time graph, which should show no significant
glitches while loading
'''

import sys
import libavg
from libavg import player

APP_RESOLUTION = (640, 480)


class AsyncLoadApp(libavg.AVGApp):
    def init(self):
        '''
        Create placeholders for the example. A single ImageNode is used to show
        the pictures.
        '''
        self.__imageNode = libavg.avg.ImageNode(pos=(10, 20), parent=self._parentNode)
        self.__spinner = libavg.avg.RectNode(color='222222',
                fillopacity=1, size=(40, 40), active=False,
                pos=(10, self._parentNode.size.y - 50), parent=self._parentNode)
        self.__infoNode = libavg.avg.WordsNode(text='Press space to load the first image',
                fontsize=11, pos=(10, 5), parent=self._parentNode)
        
        self.__pics = sys.argv[1:]
        self.__currentPic = -1
        player.subscribe(player.ON_FRAME, self.__onFrame)
    
    def onKeyDown(self, event):
        '''
        Intercept a space keypress and trigger the request.
        '''
        if event.keystring == 'space':
            self.__requestNextBitmap()
    
    def __requestNextBitmap(self):
        '''
        Ask the BitmapManager to load a new file. loadBitmap() call returns immediately.
        '''
        self.__currentPic = (self.__currentPic + 1) % len(self.__pics)
        libavg.avg.BitmapManager.get().loadBitmap(self.__pics[self.__currentPic],
                self.__onBitmapLoaded)
                
        self.__spinner.active = True
        self.__spinner.angle = 0
        
    def __onBitmapLoaded(self, bmp):
        '''
        This callback is invoked by BitmapManager, 'bmp' can be either a Bitmap instance
        or a RuntimeError instance (hence checking for Exception is consistent).
        '''
        self.__spinner.active = False
        if isinstance(bmp, Exception):
            self.__infoNode.text = ('Error loading '
                    'image %s : %s' % (self.__pics[self.__currentPic], str(bmp)))
            self.__imageNode.href = ''
        else:
            self.__infoNode.text = ('Loaded %s, '
                    'press space for the next one' % self.__pics[self.__currentPic])
            self.__setBitmapAndResize(bmp)
    
    def __setBitmapAndResize(self, bmp):
        originalSize = bmp.getSize()
        
        if originalSize.x > originalSize.y:
            ratio = (APP_RESOLUTION[0] - 20) / originalSize.x
        else:
            ratio = (APP_RESOLUTION[1] - 40) / originalSize.y
        
        self.__imageNode.setBitmap(bmp)
        self.__imageNode.size = originalSize * ratio
            
    def __onFrame(self):
        if self.__spinner.active:
            self.__spinner.angle += 0.05

if len(sys.argv) == 1:
    print 'Usage: %s <filename> [<filename> [<filename> [..]]]' % sys.argv[0]
    sys.exit(1)
    
AsyncLoadApp.start(resolution=APP_RESOLUTION)

