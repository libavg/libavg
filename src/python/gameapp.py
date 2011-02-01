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
# Original author of this file is OXullo Intersecans <x@brainrapers.org>
#

'''
Transitional class to fulfill some of the missing features of AVGApp,
currently needed for multitouch games.
'''

import os
import sys
import libavg
from libavg import avg

g_Player = avg.Player.get()
g_Log = avg.Logger.get()

_app = None

def app():
    global _app
    return _app


ownStarter = False

class GameApp(libavg.AVGApp):
    '''
    Derivation from this class adds a command line parser, which can be used to
    define fullscreen behavior and resolution. It defaults to fullscreen and
    sets the resolution as the current desktop's one.
    Multitouch is enabled by default.
    
    The variable ownStarter is set to True when the application has been started
    by commandline, opposing as started by an appChooser.
    '''
    multitouch = True

    def __init__(self, *args, **kwargs):
        avg.WordsNode.addFontDir(libavg.AVGAppUtil.getMediaDir(__file__, 'fonts'))
        global _app
        _app = self
        super(GameApp, self).__init__(*args, **kwargs)
        self._parentNode.mediadir = libavg.AVGAppUtil.getMediaDir(__file__)

    @classmethod
    def start(cls, *args, **kwargs):
        import optparse

        parser = optparse.OptionParser()
        parser.add_option('-r', '--resolution', dest='resolution',
                default=None, help='set an explicit resolution', metavar='WIDTHxHEIGHT')
        parser.add_option('-w', '--window', dest='window', action='store_true',
                default=False, help='run the game in a window')

        (options, args) = parser.parse_args()

        if options.resolution is not None:
            import re

            m = re.match('^(\d+)x(\d+)$', options.resolution)

            if m is None:
                sys.stderr.write('\n** ERROR: invalid resolution '
                        'specification %s\n\n' % options.resolution)
                parser.print_help()
                sys.exit(1)
            else:
                kwargs['resolution'] = map(int, m.groups())
        elif not 'resolution' in kwargs:
            kwargs['resolution'] = g_Player.getScreenResolution()

        if options.window:
            if options.resolution is None:
                sys.stderr.write('\n** ERROR: in window mode the resolution '
                        'must be set\n\n')
                parser.print_help()
                sys.exit(1)
            else:
                if 'AVG_DEPLOY' in os.environ:
                    del os.environ['AVG_DEPLOY']
        else:
            os.environ['AVG_DEPLOY'] = '1'

        g_Log.trace(g_Log.APP, 'Setting resolution to: %s' % str(kwargs['resolution']))

        global ownStarter
        ownStarter = True
        
        super(GameApp, cls).start(*args, **kwargs)


    def getUserdataPath(self, fname):
        '''
        Returns a path which is platform dependent and that points to a directory
        which can be used to store games data.
        These data will belong to the current user and are not meant for system-wide
        settings.
        The path is constructed using the name of the app's class and it is created
        if it doesn't exist.
        '''
        if os.name == 'posix':
            path = os.path.join(os.environ['HOME'], '.avg',
                    self.__class__.__name__.lower())
        elif os.name == 'nt':
            path = os.path.join(os.environ['APPDATA'], 'Avg',
                    self.__class__.__name__.lower())
        else:
            raise RuntimeError('Unsupported system %s' % os.name)

        try:
            os.makedirs(path)
        except OSError, e:
            import errno
            if e.errno != errno.EEXIST:
                raise

        return os.path.join(path, fname)
