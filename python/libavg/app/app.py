#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2020 Ulrich von Zadow
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
# Original author of this file is OXullo Intersecans <x at brainrapers dot org>

import os
import math

import libavg
from libavg import avg, player, Point2D, mtemu, clicktest

import settings
from settings import Option
import keyboardmanager
import debugpanel
import flashmessage
import touchvisualization


class MainDiv(avg.DivNode):
    VERSION = 'undef'

    def __init__(self, **kargs):
        assert 'parent' not in kargs
        super(MainDiv, self).__init__(**kargs)
        self.registerInstance(self, None)
        self.__touchVisOverlay = None

    def toggleTouchVisualization(self, visClass=touchvisualization.TouchVisualization):
        if self.__touchVisOverlay is not None:
            self.__touchVisOverlay.unlink(True)
            del self.__touchVisOverlay
            self.__touchVisOverlay = None
        else:
            self.__touchVisOverlay = touchvisualization.TouchVisualizationOverlay(
                    isDebug=False, visClass=visClass, size=self.size,
                    parent=self)

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
        self._clickTest = None

        self._settings = None
        self._setupSettings()

    def run(self, mainDiv, **kargs):
        assert isinstance(mainDiv, MainDiv)
        self._mainDiv = mainDiv

        mainDiv.settings = self._settings
        self._applySettingsExtenders(kargs)
        self._setupLogging()

        self._startupMainDiv()

        self._setupResolution()
        self._setupRootNode()
        self._setupMultisampling()
        self._setupMouse()
        self._setupVolume()
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

        try:
            self._runLoop()
        except Exception:
            self._stopClickTest()
            self._teardownKeyboardManager()
            raise

        self._exitMainDiv()

        self._stopClickTest()
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
        screenBmp = player.screenshot()

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
        objects = player.getTestHelper().getObjectCount()
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
            raise avg.Exception('%s has been already instantiated' %
                    self.__class__.__name__)

        libavg.app.instance = self

    def _setupSettings(self):
        self._settings = settings.Settings()
        self._settings.addOption(Option('app_windowconfig', ''))
        self._settings.addOption(Option('app_resolution', '640x480'))
        self._settings.addOption(Option('app_window_size', ''))
        self._settings.addOption(Option('app_fullscreen', 'false'))
        self._settings.addOption(Option('app_multisample_samples', '8'))
        self._settings.addOption(Option('app_show_cursor', 'true'))
        self._settings.addOption(Option('app_rotation', 'normal'))
        self._settings.addOption(Option('app_panel_fontsize', '10'))
        self._settings.addOption(Option('app_mouse_enabled', 'true'))
        self._settings.addOption(Option('app_volume', '1'))
        self._settings.addOption(Option('tuio_enabled', 'false'))
        self._settings.addOption(Option('tuio_port', '3333'))
        self._settings.addOption(Option('log_avg_categories', ''))

    def _applySettingsExtenders(self, kargs):
        self._settings.applyExtender(settings.KargsExtender(kargs))
        argvExtender = settings.ArgvExtender(self._mainDiv.VERSION)
        self._mainDiv.onArgvParserCreated(argvExtender.parser)
        self._settings.applyExtender(argvExtender)
        self._mainDiv.onArgvParsed(argvExtender.parsedArgs[0], argvExtender.parsedArgs[1],
                argvExtender.parser)

    def _setupLogging(self):
        catMap = self._settings.get('log_avg_categories').strip()
        if catMap:
            for catPair in catMap.split(' '):
                cat, strLevel = catPair.split(':')
                level = getattr(avg.logger.Severity, strLevel)
                avg.logger.configureCategory(cat, level)

    def _startupMainDiv(self):
        self._mainDiv.onStartup()

    def _setupRootNode(self):
        # FIXME: "../../libavg/doc/avg.dtd" doesn't exist (anymore)
        player.loadString('''<?xml version="1.0"?>
        <!DOCTYPE avg SYSTEM "../../libavg/doc/avg.dtd">
        <avg width="%s" height="%s">
        </avg>''' % tuple(self._resolution))

    def _setupMultisampling(self):
        samples = self._settings.getInt('app_multisample_samples')
        player.setMultiSampleSamples(samples)

    def _setupMouse(self):
        player.enableMouse(self._settings.getBoolean('app_mouse_enabled'))
        player.showCursor(self._settings.getBoolean('app_show_cursor'))

    def _setupVolume(self):
        player.volume = self._settings.getFloat('app_volume')

    def _setupTUIO(self):
        if self._settings.getBoolean('tuio_enabled'):
            os.putenv('AVG_ENABLE_TUIO', '1')

            tuio_port = self._settings.get('tuio_port')
            if tuio_port:
                os.putenv('AVG_TUIO_PORT', tuio_port)

    def _getAppParentGeometry(self):
        rotation = self._settings.get('app_rotation').lower()
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
            raise ValueError('Invalid rotation %s' % rotation)

        return pos, size, angle

    def _setupAppParent(self, pos, size, angle):
        self._appParent = avg.DivNode(parent=player.getRootNode(),
                pos=pos, size=size, angle=angle)

    def _setupMainDiv(self):
        self._appParent.appendChild(self._mainDiv)
        self._mainDiv.size = self._appParent.size

    def _setupTopPanel(self):
        self._overlayPanel = avg.DivNode(parent=self._appParent, id='overlayPanel')

    def _setupDebugPanel(self):
        self._debugPanel = debugpanel.DebugPanel(parent=self._appParent,
                size=self._appParent.size, id='debugPanel',
                fontsize=self._settings.getFloat('app_panel_fontsize'))

    def _setupDebuggingWidgets(self):
        pass

    def _setupResolution(self):
        rotation = self._settings.get('app_rotation').lower()
        resolutionStr = self._settings.get('app_resolution').lower()
        windowSizeStr = self._settings.get('app_window_size')
        if self._settings.get('app_windowconfig') == '':
            if resolutionStr != '':
                resolution = self._settings.getPoint2D('app_resolution')
            else:
                resolution = player.getScreenResolution()

            if windowSizeStr != '':
                windowSize = self._settings.getPoint2D('app_window_size')
            else:
                windowSize = resolution

            if rotation in ('left', 'right'):
                resolution = Point2D(resolution.y, resolution.x)
                windowSize = Point2D(windowSize.y, windowSize.x)

            self._resolution = resolution
            self._windowSize = windowSize
        else:
            if rotation != 'normal' or windowSizeStr != '':
                raise ValueError('App parameters: app_windowconfig is incompatible '
                        'with app_window_size and app_rotation')
            player.setWindowConfig(self._settings.get('app_windowconfig'))
            self._resolution = self._settings.getPoint2D('app_resolution')

    def _applyResolution(self):
        if self._settings.get('app_windowconfig') == '':

            fullscreen = self._settings.getBoolean('app_fullscreen')

            if fullscreen:
                resolution = self._resolution
            else:
                resolution = self._windowSize

            player.setResolution(
                    fullscreen,
                    int(resolution.x), int(resolution.y),
                    0)  # color depth

    def _setupKeyboardManager(self):
        keyboardmanager.init()

        keyboardmanager.bindKeyDown(
                keyname='D',
                handler=self._debugPanel.toggleVisibility,
                help='Show/hide the debug panel',
                modifiers=avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keyname='H',
                handler=lambda: player.showCursor(not player.isCursorShown()),
                help='Show/hide cursor',
                modifiers=avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keyname='P',
                handler=self.takeScreenshot,
                help='Take screenshot',
                modifiers=avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keyname='B',
                handler=self.dumpTextObjectCount,
                help='Dump objects count to the console',
                modifiers=avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keyname='E',
                handler=self._toggleMtEmulation,
                help='Toggle multitouch emulation',
                modifiers=avg.KEYMOD_CTRL)

        keyboardmanager.bindKeyDown(
                keyname='C',
                handler=self._toggleClickTest,
                help='Toggle click test',
                modifiers=avg.KEYMOD_CTRL)

        self.debugPanel.setupKeys()

    def _toggleMtEmulation(self):
        if self._mtEmu is None:
            self._mtEmu = mtemu.MTemu()
            keyboardmanager.bindKeyDown(
                    keyname='Shift',
                    handler=self._mtEmu.enableDualTouch,
                    help='Enable pinch gesture emulation',
                    # NOTE: modifier required because Shift itself is a modifier
                    modifiers=keyboardmanager.KEYMOD_ANY)
            keyboardmanager.bindKeyUp(
                    keyname='Shift',
                    handler=self._mtEmu.disableDualTouch,
                    help='Disable pinch gesture emulation')

            keyboardmanager.bindKeyDown(
                    keyname='T',
                    handler=self._mtEmu.toggleSource,
                    help='Toggle source between TOUCH and TRACK',
                    modifiers=avg.KEYMOD_CTRL)
        else:
            self._mtEmu.deinit()
            keyboardmanager.unbindKeyDown(
                    keyname='T',
                    modifiers=avg.KEYMOD_CTRL)
            keyboardmanager.unbindKeyDown(
                    keyname='Shift',
                    modifiers=keyboardmanager.KEYMOD_ANY)
            keyboardmanager.unbindKeyUp(keyname='Shift')

            del self._mtEmu
            self._mtEmu = None

    def _toggleClickTest(self):
        if not self._clickTest:
            self._clickTest = clicktest.ClickTest()
            self._clickTest.start()
        else:
            self._stopClickTest()

    def _stopClickTest(self):
        if self._clickTest:
            self._clickTest.stop()
            self._clickTest = None

    def _teardownKeyboardManager(self):
        keyboardmanager.unbindAll()

    def _setupOnInit(self):
        self._setupTUIO()
        player.setTimeout(0, self._onInitInternal)

    def _runLoop(self):
        player.play()

    def _onInitInternal(self):
        self._mainDiv.onInit()
        player.subscribe(player.ON_FRAME, self._mainDiv.onFrame)

    def _exitMainDiv(self):
        self._mainDiv.onExit()
