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


import os
import math
import time

import libavg
from libavg import avg, Point2D, mtemu

import settings
from settings import Option
import keyboardmanager
import debugpanel
import flashmessage


class MainDiv(libavg.avg.DivNode):
    INIT_FUNC = 'onInit'
    VERSION = 'undef'

    def __init__(self, **kargs):
        assert not 'parent' in kargs
        super(MainDiv, self).__init__(**kargs)
        self.registerInstance(self, None)

    def onArgvParserCreated(self, parser):
        pass

    def onArgvParsed(self, options, args, parser):
        pass

    def onStartup(self):
        pass

    def onInit(self):
        pass

    def onExit(self):
        pass

    def onFrame(self):
        pass


class App(object):
    def __init__(self):
        self._setupInstance()

        self._mainDiv = None
        self._appParent = None
        self._debugPanel = None
        self._overlayPanel = None
        self._resolution = None
        self._windowSize = None
        self._mtEmu = None

        self.__lastFrameTimestamp = 0

        self._setupSettings()

    def run(self, mainDiv, **kargs):
        assert isinstance(mainDiv, MainDiv)
        self._mainDiv = mainDiv

        self.mainDiv.settings = self._settings
        self._applySettingsExtenders(kargs)
        self._setupLogging()

        mainDiv.onStartup()

        self._setupResolution()
        self._setupRootNode()
        self._setupMouse()
        pos, size, angle = self._getAppParentGeometry()
        self._setupAppParent(pos, size, angle)
        self._setupMainDiv()
        self._setupTopPanel()

        self._setupDebugPanel()
        self._setupKeyboardManager()
        self._setupDebuggingWidgets()
        self._applyResolution()
        self._setupOnInit()

        self.onBeforeLaunch()

        self.__lastFrameTimestamp = time.time()

        try:
            self._runLoop()
        except Exception, e:
            self._teardownKeyboardManager()
            raise

        mainDiv.onExit()

        self._teardownKeyboardManager()

        return 0

    @property
    def mainDiv(self):
        return self._mainDiv

    @property
    def debugPanel(self):
        return self._debugPanel

    @property
    def overlayPanel(self):
        return self._overlayPanel

    @property
    def settings(self):
        return self._settings

    def onBeforeLaunch(self):
        pass

    def takeScreenshot(self, targetFolder='.'):
        screenBmp = libavg.player.screenshot()

        filenameTemplate = os.path.join(targetFolder, '%s-%03d.png')

        i = 1
        while i < 1000:
            filename = filenameTemplate % (self.__class__.__name__, i)
            if os.path.exists(filename):
                i += 1
            else:
                break

        if i == 1000:
            flashmessage.FlashMessage('Maximum number of screenshots reached',
                    parent=self._appParent, isError=True)
        else:
            screenBmp.save(filename)
            flashmessage.FlashMessage('Screenshot saved as %s' % filename,
                    parent=self._appParent)

    def dumpTextObjectCount(self):
        objects = libavg.player.getTestHelper().getObjectCount()
        savedSeverity = libavg.logger.getCategories()[libavg.logger.Category.APP]
        libavg.logger.configureCategory(libavg.logger.Category.APP,
                libavg.logger.Severity.INFO)
        libavg.logger.info('Dumping objects count')
        for key, value in objects.iteritems():
            libavg.logger.info('  %-25s: %s' % (key, value))

        libavg.logger.configureCategory(libavg.logger.Category.APP, savedSeverity)

    def _setupInstance(self):
        import libavg.app

        if libavg.app.instance is not None:
            raise RuntimeError('%s has been already instantiated' %
                    self.__class__.__name__)

        libavg.app.instance = self

    def _setupSettings(self):
        self._settings = settings.Settings()
        self._settings.addOption(Option('app_resolution', '640x480'))
        self._settings.addOption(Option('app_window_size', ''))
        self._settings.addOption(Option('app_fullscreen', 'false'))
        self._settings.addOption(Option('app_show_cursor', 'true'))
        self._settings.addOption(Option('app_rotation', 'normal'))
        self._settings.addOption(Option('app_panel_fontsize', '10'))
        self._settings.addOption(Option('app_mouse_enabled', 'true'))
        self._settings.addOption(Option('multitouch_enabled', 'false'))
        self._settings.addOption(Option('multitouch_driver', ''))
        self._settings.addOption(Option('multitouch_tuio_port', ''))
        self._settings.addOption(Option('multitouch_mtdev_device', ''))
        self._settings.addOption(Option('log_avg_categories', ''))

    def _applySettingsExtenders(self, kargs):
        self.settings.applyExtender(settings.KargsExtender(kargs))
        argvExtender = settings.ArgvExtender(self.mainDiv.VERSION)
        self.mainDiv.onArgvParserCreated(argvExtender.parser)
        self.settings.applyExtender(argvExtender)
        self.mainDiv.onArgvParsed(argvExtender.parsedArgs[0], argvExtender.parsedArgs[1],
                argvExtender.parser)

    def _setupLogging(self):
        catMap = self.settings.get('log_avg_categories').strip()
        if catMap:
            for catPair in catMap.split(' '):
                cat, strLevel = catPair.split(':')
                level = getattr(avg.logger.Severity, strLevel)
        
                libavg.avg.logger.configureCategory(cat, level)

    def _setupRootNode(self):
        libavg.player.loadString('''<?xml version="1.0"?>
        <!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
        <avg width="%s" height="%s">
        </avg>''' % tuple(self._resolution))

    def _setupMouse(self):
        libavg.player.enableMouse(self.settings.getBoolean('app_mouse_enabled'))

    def _setupMultitouch(self):
        if self.settings.getBoolean('multitouch_enabled'):
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
        size = self._resolution
        pos = (0, 0)
        angle = 0

        if rotation == 'left':
            angle = -math.pi / 2
            size = (self._resolution.y, self._resolution.x)
            pos = ((self._resolution.x - self._resolution.y) / 2,
                    (self._resolution.y - self._resolution.x) / 2)
        elif rotation == 'right':
            angle = math.pi / 2
            size = (self._resolution.y, self._resolution.x)
            pos = ((self._resolution.x - self._resolution.y) / 2,
                    (self._resolution.y - self._resolution.x) / 2)
        elif rotation == 'inverted':
            angle = math.pi
        elif rotation != 'normal':
            raise TypeError('Invalid rotation %s' % rotation)

        return (pos, size, angle)

    def _setupAppParent(self, pos, size, angle):
        self._appParent = libavg.avg.DivNode(parent=libavg.player.getRootNode(),
                pos=pos, size=size, angle=angle)

    def _setupMainDiv(self):
        self._appParent.appendChild(self.mainDiv)
        self.mainDiv.size = self._appParent.size

    def _setupTopPanel(self):
        self._overlayPanel = libavg.avg.DivNode(parent=self._appParent, id='overlayPanel')

    def _setupDebugPanel(self):
        self._debugPanel = debugpanel.DebugPanel(parent=self._appParent,
                    size=self._appParent.size, id='debugPanel',
                    fontsize=self.settings.getFloat('app_panel_fontsize'))

    def _setupDebuggingWidgets(self):
        pass

    def _setupResolution(self):
        rotation = self.settings.get('app_rotation').lower()
        resolutionStr = self.settings.get('app_resolution').lower()
        if resolutionStr != '':
            resolution = self.settings.getPoint2D('app_resolution')
        else:
            resolution = libavg.player.getScreenResolution()

        windowSizeStr = self.settings.get('app_window_size')
        if windowSizeStr != '':
            windowSize = self.settings.getPoint2D('app_window_size')
        else:
            windowSize = resolution

        if rotation in ('left', 'right'):
            resolution = Point2D(resolution.y, resolution.x)
            windowSize = Point2D(windowSize.y, windowSize.x)

        self._resolution = resolution
        self._windowSize = windowSize

    def _applyResolution(self):
        fullscreen = self.settings.getBoolean('app_fullscreen')

        if fullscreen:
            resolution = self._resolution
        else:
            resolution = self._windowSize

        libavg.player.setResolution(
                fullscreen,
                int(resolution.x), int(resolution.y),
                0  # color depth
                )

        libavg.player.showCursor(self.settings.getBoolean('app_show_cursor'))

    def _setupKeyboardManager(self):
        keyboardmanager.init()
        keyboardmanager.bindKeyDown(
                keystring='d',
                handler=self._debugPanel.toggleVisibility,
                help='Show/hide the debug panel',
                modifiers=libavg.avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keystring='h',
                handler=lambda: libavg.player.showCursor(
                        not libavg.player.isCursorShown()),
                help='Show/hide cursor',
                modifiers=libavg.avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keystring='p',
                handler=self.takeScreenshot,
                help='Take screenshot',
                modifiers=libavg.avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keystring='b',
                handler=self.dumpTextObjectCount,
                help='Dump objects count to the console',
                modifiers=libavg.avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keystring='e',
                handler=self._toggleMtEmulation,
                help='Toggle multitouch emulation',
                modifiers=libavg.avg.KEYMOD_CTRL)

        self.debugPanel.setupKeys()

    def _toggleMtEmulation(self):
        if self._mtEmu is None:
            self._mtEmu = mtemu.MTemu()
            keyboardmanager.bindKeyDown('shift', self._mtEmu.enableDualTouch,
                    'Enable pinch gesture emulation')
            keyboardmanager.bindKeyUp('shift', self._mtEmu.disableDualTouch,
                    'Disable pinch gesture emulation')

            keyboardmanager.bindKeyDown('t', self._mtEmu.toggleSource,
                    'Toggle source between TOUCH and TRACK', libavg.avg.KEYMOD_CTRL)
        else:
            self._mtEmu.deinit()
            keyboardmanager.unbindKeyDown('t', libavg.avg.KEYMOD_CTRL)
            keyboardmanager.unbindKeyDown('shift')
            keyboardmanager.unbindKeyUp('shift')

            del self._mtEmu
            self._mtEmu = None

    def _teardownKeyboardManager(self):
        keyboardmanager.unbindAll()

    def _setupOnInit(self):
        libavg.player.setTimeout(0, self._onInitInternal)

    def _runLoop(self):
        libavg.player.play()

    def _onInitInternal(self):
        self._setupMultitouch()
        initFunc = getattr(self.mainDiv, self.mainDiv.INIT_FUNC)
        initFunc()
        libavg.player.subscribe(libavg.player.ON_FRAME, self.mainDiv.onFrame)
