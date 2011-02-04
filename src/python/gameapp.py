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

from __future__ import with_statement
import os
import sys
import cPickle as pickle

import libavg
from libavg import avg

g_Player = avg.Player.get()
g_Log = avg.Logger.get()

_app = None

def app():
    global _app
    return _app


class Datastore(object):
    '''
    General purpose persistent object (state in self.data)
    
    A possible use case:
    
    >>> from libavg import gameapp
    
    >>> def validate(data):
    >>>    return type(data) == list and isinstance(data[2], Bar)

    >>> myInitialData = [1, 'foo', Bar(12)]
    >>> myDs = gameapp.Datastore('hiscore', myInitialData, validate)
    >>> myDs.data[2].foobarize(1)
    >>> myDs.data[0] = 2
    >>> myDs.commit()
    
    >>> againMyDs = gameapp.Datastore.get('hiscore')
    '''
    
    instances = {}
    
    def __init__(self, tag, initialData=None, validator=lambda ds: True, autoCommit=True):
        '''
        tag: an application-wise unique string which is used to key the datastore and
                set its dump filename.
        initialData: a callable (class, method) or an instance that is used to initialize
                the datastore when the dumpfile either doesn't exist, it's corrupted, or
                cannot be accessed or when the loaded data don't comply with the optional
                validator. Defaults to an empty dict.
        validator: a callable to which stored data are passed and that is able to
                determine if the data format is expected. If the call returns True, the
                proposed data are accepted and set. With False as return value the
                Datastore uses initialData instead.
        autoCommit: if True, the datastore sync itself when the program exits (uses
                atexit). It's always possible to commit() manually.
        '''
        self.__tag = tag
        self.__dumpFile = app().getUserdataPath(tag + '.pkl')

        if tag in Datastore.instances:
            raise RuntimeError('%s already initialized' % self)
        
        if hasattr(initialData, '__call__'):
            initialData = initialData()
        elif initialData is None:
            initialData = dict()
        
        if os.path.exists(self.__dumpFile):
            if not os.path.isfile:
                raise RuntimeError('%s dump file '
                        'is not a plain file' % self)
            elif not os.access(self.__dumpFile, os.R_OK | os.W_OK):
                raise RuntimeError('%s dump file'
                        'cannot be accessed with r/w permissions' % self)

        try:
            f = open(self.__dumpFile)
        except IOError:
            g_Log.trace(g_Log.APP, 'Initializing %s' % self)
            self.data = initialData
            self.commit()
        else:
            try:
                self.data = pickle.load(f)
            except:
                f.close()
                g_Log.trace(g_Log.ERROR, 'Datastore %s is corrupted, '
                        'reinitializing' % self)
                self.data = initialData
                self.commit()
            else:
                f.close()
                if not validator(self.data):
                    g_Log.trace(g_Log.ERROR, 'Sanity check failed for %s: '
                            'reinitializing' % self)
                    self.data = initialData
                    self.commit()
                else:
                    g_Log.trace(g_Log.APP, '%s successfully '
                            'loaded' % self)
        
        if autoCommit:
            import atexit
            atexit.register(self.commit)
        
        Datastore.instances[tag] = self
    
    @staticmethod
    def get(tag):
        '''
        Retrieve a Datastore using its tag
        '''
        return Datastore.instances.get(tag, None)
        
    def commit(self):
        '''
        Dump Datastore data to disk
        '''
        import time
        
        tempFile = self.__dumpFile + '.tmp.' + str(int(time.time() * 1000))

        try:
            with open(tempFile, 'wb') as f:
                pickle.dump(self.data, f)
        except Exception, e:
            g_Log.trace(g_Log.ERROR, 'Cannot save '
                    '%s (%s)' % (self.__dumpFile, str(e)))
            return False
        else:
            if os.path.exists(self.__dumpFile):
                try:
                    os.remove(self.__dumpFile)
                except Exception, e:
                    g_Log.trace(g_Log.ERROR, 'Cannot overwrite '
                            'dump file %s (%s)' % (self, str(e)))
                    return False
            try:
                os.rename(tempFile, self.__dumpFile)
            except Exception, e:
                g_Log.trace(g_Log.ERROR, 'Cannot save '
                        '%s (%s)' % (self, str(e)))
                os.remove(tempFile)
                return False
            else:
                g_Log.trace(g_Log.APP, '%s saved' % self)
                return True
    
    def __repr__(self):
        return '<%s:%s %s>' % (self.__class__.__name__, self.__tag, self.__dumpFile)


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
        global _app
        _app = self
        super(GameApp, self).__init__(*args, **kwargs)
        
        pkgpath = self._getPackagePath()
        if pkgpath is not None:
            avg.WordsNode.addFontDir(libavg.AVGAppUtil.getMediaDir(pkgpath, 'fonts'))
            self._parentNode.mediadir = libavg.AVGAppUtil.getMediaDir(pkgpath)

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
        Return a path which is platform dependent and that points to a directory
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

    def _getPackagePath(self):
        '''
        Overload this method in your App class if you want to get 'media' and 'fonts'
        easily set up (respectively as mediadir for App._parentNode and for local font
        path).
        This method should return a relative (to CWD) or absolute path. It can be a file
        path as well.
        Eg:
        
        def _getPackagePath(self):
            return __file__
        '''
        return None
