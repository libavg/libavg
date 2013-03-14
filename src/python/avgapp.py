# libavg - Media Playback Engine.
# Copyright (C) 2003-2011 Ulrich von Zadow
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
# Original author of this file is Martin Heistermann <mh at sponc dot de>
#

from appstarter import AppStarter


class AVGApp(object):
    _instances = {}
    multitouch = False
    fakeFullscreen = False

    def __init__(self, parentNode):
        '''
        Initialization before Player.play()
        Use this only when needed, e.g. for
        WordsNode.addFontDir(). Do not forget to call
        super(YourApp, self).__init__(parentNode)
        '''

        import warnings
        warnings.warn('AVGApp is deprecated, use libavg.app.App instead')

        appname = self.__class__.__name__
        if appname in AVGApp._instances:
            raise RuntimeError('App %s already setup' % appname)
            
        AVGApp._instances[appname] = self

        self.__isRunning = False
        self._parentNode = parentNode
        self._starter = None

        if 'onKey' in dir(self):
            raise DeprecationWarning, \
                    'AVGApp.onKey() has been renamed to AVGApp.onKeyDown().'

    @classmethod
    def get(cls):
        '''
        Get the Application instance
        
        Note: this class method has to be called from the top-level app class:

        >>> class MyApp(libavg.AVGApp):
        ...  pass
        >>> instance = MyApp.get()
        '''
        return cls._instances.get(cls.__name__, None)

    @classmethod
    def start(cls, **kwargs):
        if cls.multitouch:
            from appstarter import AVGMTAppStarter
            starter = AVGMTAppStarter
        else:
            from appstarter import AVGAppStarter
            starter = AVGAppStarter
        
        starter(appClass=cls, fakeFullscreen=cls.fakeFullscreen, **kwargs)

    def init(self):
        """main initialization
        build node hierarchy under self.__parentNode."""
        pass

    def exit(self):
        """Deinitialization
        Called after player.play() returns. End of program run."""
        pass

    def _enter(self):
        """enter the application, internal interface.
        override this and start all animations, intervals
        etc. here"""
        pass

    def _leave(self):
        """leave the application, internal interface.
        override this and stop all animations, intervals
        etc. Take care your application does not use any
        non-needed resources after this."""
        pass

    def enter(self, onLeave = lambda: None):
        """enter the application, external interface.
        Do not override this."""
        self.__isRunning = True
        self._onLeave = onLeave
        self._enter()

    def leave(self):
        """leave the application, external interface.
        Do not override this."""
        self.__isRunning = False
        self._onLeave()
        self._leave()

    def onKeyDown(self, event):
        """returns bool indicating if the event was handled
        by the application """
        return False

    def onKeyUp(self, event):
        """returns bool indicating if the event was handled
        by the application """
        return False

    def isRunning(self):
        return self.__isRunning

    def setStarter(self, starter):
        self._starter = starter

    def getStarter(self):
        return self._starter


class App(object):
    @classmethod
    def start(cls, *args, **kargs):
        raise RuntimeError('avgapp.App cannot be used any longer. Use libavg.AVGApp for '
                'a compatible class or switch to the new libavg.app.App')

