#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de
#
# Original author of this file is OXullo Interecans <x at brainrapers dot org>
# Sponsored by Archimedes Exhibitions GmbH ( http://www.archimedes-exhibitions.de )


import os
import io
import math
import time

import libavg
from libavg import avg, Point2D

#import loghelpers
import settings
from settings import Option
import keyboardmanager
#import debugpanel
import flashmessage


class MainScene(libavg.avg.DivNode):
    '''
    Main construction block for libavg-based applications
    '''
    INIT_FUNC = 'onInit'

    def __init__(self, **kargs):
        assert not 'parent' in kargs
        super(MainScene, self).__init__(**kargs)
        self.registerInstance(self, None)

    def onStartup(self):
        '''
        Called before libavg has been setup
        Setup early services or communication channels here
        '''
        pass

    def onInit(self):
        '''
        Called by a libavg timer as soon as the main loop starts
        Build the application node tree here
        '''
        pass

    def onExit(self):
        '''
        Called when the main loop stops
        Release resources and cleanup here
        '''
        pass

    def onFrame(self, delta):
        '''
        Called at every frame
        @param delta: time in seconds since the last onFrame() call
        @type delta: float
        '''
        pass


class AppSettings(settings.Settings):
    DEFAULTS = [
            Option('app_resolution', '640x480'),
            Option('app_window_size', '640x480'),
            Option('app_fullscreen', 'false'),
            Option('app_show_cursor', 'true'),
            Option('app_rotation', 'normal'),
            Option('app_panel_fontsize', '10'),
            Option('app_mouse_enabled', 'true'),
            Option('multitouch_enabled', 'false'),
            Option('multitouch_driver', ''),
            Option('multitouch_tuio_port', ''),
            Option('multitouch_mtdev_device', ''),
#            Option('logging_sink', 'libavg'),
#            Option('logging_severity', 'INFO'),
#            Option('logging_categories', ''),
    ]
    
    def __init__(self, defaults=[]):
        super(AppSettings, self).__init__(self.DEFAULTS + defaults)


class App(object):
    '''
    libavg-based application class
    '''

    def __init__(self, settingsInstance=None):
        import libavg.app

        if libavg.app.instance is not None:
            raise RuntimeError('%s has been already instantiated' %
                    self.__class__.__name__)

        libavg.app.instance = self

        self._mainScene = None
        self._appParent = None
        self._debugPanel = None
        self._overlayPanel = None
        self._resolution = None
        self._windowSize = None

        self.__lastFrameTimestamp = 0

        if settingsInstance is None:
            self._setupSettings()
        else:
            self._settings = settingsInstance

        #self._setupLogging()

    def run(self, mainScene, **kargs):
        '''
        Start the application using the provided L{MainScene} instance

        @param mainScene: an instance of L{MainScene} (or derived class)
        '''
        assert isinstance(mainScene, MainScene)
        self._mainScene = mainScene

        self._applySettingsExtenders(kargs)

        mainScene.onStartup()

        self._setupResolution()
        self._setupRootNode()
        self._setupMouse()
        self._setupMultitouch()
        pos, size, angle = self._getAppParentGeometry()
        self._setupAppParent(pos, size, angle)
        self._setupMainScene()
        self._setupTopPanel()

        #self._setupDebugPanel()
        self._setupKeyboardManager()
        #self._setupDebuggingWidgets()
        self._applyResolution()
        self._setupOnInit()

        self.onBeforeLaunch()

        self.__lastFrameTimestamp = time.time()

        self._runLoop()

        mainScene.onExit()
        
        self._teardownKeyboardManager()

        return 0

    def stop(self):
        libavg.player.stop()

    @property
    def mainScene(self):
        '''
        MainScene instance passed to method L{run}
        '''
        return self._mainScene

    @property
    def appParent(self):
        '''
        Base DivNode where the application sets up the control layers
            (L{MainScene}, debug, messaging)
        '''
        return self._appParent

#    @property
#    def debugPanel(self):
#        '''
#        The instance of L{app.debugpanel.DebugPanel} that currently runs in the
#            application
#        '''
#        return self._debugPanel

    @property
    def overlayPanel(self):
        '''
        DivNode that stands on top of the MainScene
        '''
        return self._overlayPanel

    @property
    def settings(self):
        '''
        The instance of L{app.settings.Settings} associated to the application
        '''
        return self._settings

    @property
    def resolution(self):
        '''
        Current target screen resolution
        '''
        return self._resolution

    @property
    def windowSize(self):
        '''
        Current target window size
        '''
        return self._windowSize

    def onBeforeLaunch(self):
        pass

    def takeScreenshot(self):
        screenBmp = libavg.player.screenshot()

        filenameTemplate = '%s-%03d.png'

        i = 1
        while i < 1000:
            filename = filenameTemplate % (self.__class__.__name__, i)
            if os.path.exists(filename):
                i += 1
            else:
                break

        if i == 1000:
            flashmessage.FlashMessage('Maximum number of screenshots reached',
                    parent=self.appParent, isError=True)
        else:
            screenBmp.save(filename)
            flashmessage.FlashMessage('Screenshot saved as %s' % filename,
                    parent=self.appParent)

    def _setupSettings(self):
        self._settings = AppSettings()

    def _applySettingsExtenders(self, kargs):
        self.settings.applyExtender(settings.KargsExtender(kargs))
        self.settings.applyExtender(settings.ArgvExtender())

    def _setupLogging(self):
        logLevel = self.settings.get('app_loglevel')
        loghelpers.init(logLevel)

    def _setupRootNode(self):
        libavg.player.loadString('''<?xml version="1.0"?>
        <!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
        <avg width="%s" height="%s">
        </avg>''' % tuple(self.resolution))

    def _setupMouse(self):
        libavg.player.enableMouse(self.settings.getboolean('app_mouse_enabled'))

    def _setupMultitouch(self):
        if self.settings.getboolean('multitouch_enabled'):
            driver = self.settings.get('multitouch_driver').upper()
            if driver:
                os.putenv('AVG_MULTITOUCH_DRIVER', driver)

            tuio_port = self.settings.get('multitouch_tuio_port').upper()
            if tuio_port:
                os.putenv('AVG_TUIO_PORT', tuio_port)

            mtdev_device = self.settings.get('multitouch_mtdev_device').upper()
            if mtdev_device:
                os.putenv('AVG_LINUX_MULTITOUCH_DEVICE', mtdev_device)

            libavg.player.enableMultitouch()

    def _getAppParentGeometry(self):
        rotation = self.settings.get('app_rotation').lower()
        size = self.resolution
        pos = (0, 0)
        angle = 0

        if rotation == 'left':
            angle = -math.pi / 2
            size = (self.resolution.y, self.resolution.x)
            pos = ((self.resolution.x - self.resolution.y) / 2,
                    (self.resolution.y - self.resolution.x) / 2)
        elif rotation == 'right':
            angle = math.pi / 2
            size = (self.resolution.y, self.resolution.x)
            pos = ((self.resolution.x - self.resolution.y) / 2,
                    (self.resolution.y - self.resolution.x) / 2)
        elif rotation == 'inverted':
            angle = math.pi
        elif rotation != 'normal':
            raise TypeError('Invalid rotation %s' % rotation)

        return (pos, size, angle)

    def _setupAppParent(self, pos, size, angle):
        self._appParent = libavg.avg.DivNode(parent=libavg.player.getRootNode(),
                pos=pos, size=size, angle=angle)

    def _setupMainScene(self):
        self.appParent.appendChild(self.mainScene)
        self.mainScene.size = self.appParent.size

    def _setupTopPanel(self):
        self._overlayPanel = libavg.avg.DivNode(parent=self.appParent, id='overlayPanel')

#    def _setupDebugPanel(self):
#        self._debugPanel = debugpanel.DebugPanel(parent=self.appParent,
#                    size=self.appParent.size, id='debugPanel',
#                    fontsize=self.settings.getfloat('app_panel_fontsize'))

#    def _setupDebuggingWidgets(self):
#        self._debugPanel.addWidget(debugpanel.LoggerWidget)
#        self._debugPanel.addWidget(debugpanel.KeyboardManagerBindingsShower)

    def _setupResolution(self):
        rotation = self.settings.get('app_rotation').lower()
        resolution = self.settings.getpoint2d('app_resolution')
        windowSize = self.settings.getpoint2d('app_window_size')

        if rotation in ('left', 'right'):
            resolution = Point2D(resolution.y, resolution.x)
            windowSize = Point2D(windowSize.y, windowSize.x)

        self._resolution = resolution
        self._windowSize = windowSize

    def _applyResolution(self):
        fullscreen = self.settings.getboolean('app_fullscreen')

        if fullscreen:
            resolution = self.resolution
        else:
            resolution = self.windowSize

        libavg.player.setResolution(
                fullscreen,
                int(resolution.x), int(resolution.y),
                0  # color depth
                )

        libavg.player.showCursor(self.settings.getboolean('app_show_cursor'))

    def _setupKeyboardManager(self):
        keyboardmanager.init()
#        keyboardmanager.bindKeyDown(
#                keystring='d',
#                handler=self._debugPanel.toggleVisibility,
#                help='Show/hide the debug panel',
#                modifiers=libavg.avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keystring='m',
                handler=lambda: libavg.player.showCursor(
                        not libavg.player.isCursorShown()),
                help='Show/hide cursor',
                modifiers=libavg.avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keystring='p',
                handler=self.takeScreenshot,
                help='Take screenshot',
                modifiers=libavg.avg.KEYMOD_CTRL)

    def _teardownKeyboardManager(self):
        keyboardmanager.unbindAll()

    def _setupOnInit(self):
        libavg.player.setTimeout(0, self._onInitInternal)

    def _runLoop(self):
        libavg.player.play()

    def _onInitInternal(self):
        initFunc = getattr(self.mainScene, self.mainScene.INIT_FUNC)
        initFunc()
        libavg.player.subscribe(libavg.player.ON_FRAME, self._onFrameInternal)

    def _onFrameInternal(self):
        now = time.time()
        self.mainScene.onFrame(now - self.__lastFrameTimestamp)
        self.__lastFrameTimestamp = now

