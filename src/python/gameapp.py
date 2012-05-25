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
from libavg import avg, player

g_Log = avg.Logger.get()


class Datastore(object):
    '''
    General purpose persistent object (state in self.data)

    Even if it's possible to use Datastore independently from GameApp, the suggested
    use is described with the following case (given MyApp a subclass of GameApp):
    
    >>> from mypackage.app import MyApp
    >>> g_App = MyApp.get()

    >>> def validate(data):
    >>>    return type(data) == list and isinstance(data[2], Bar)

    >>> myInitialData = [1, 'foo', Bar(12)]
    >>> myDs = g_App.initDatastore('hiscore', myInitialData, validate)
    >>> myDs.data[2].foobarize(1)
    >>> myDs.data[0] = 2
    >>> myDs.commit()

    >>> againMyDs = g_App.getDatastore('hiscore')
    '''

    def __init__(self, dumpFile, initialData, validator, autoCommit):
        '''
        dumpFile: path to a file which is used for storing and retrieving data
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
        self.__dumpFile = dumpFile
        
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
        return '<%s %s>' % (self.__class__.__name__, self.__dumpFile)


class GameApp(libavg.AVGApp):
    '''
    Derivation from this class adds a command line parser, which can be used to
    define fullscreen behavior and resolution. It defaults to fullscreen and
    sets the resolution as the current desktop's one.
    Multitouch is enabled by default.
    '''
    multitouch = True
    instances = {}

    def __init__(self, *args, **kwargs):
        appname = self.__class__.__name__
        if appname in GameApp.instances:
            raise RuntimeError('App %s already setup' % appname)
            
        GameApp.instances[appname] = self
        
        super(GameApp, self).__init__(*args, **kwargs)
        
        self.__datastores = {}
        
        pkgpath = self._getPackagePath()
        if pkgpath is not None:
            avg.WordsNode.addFontDir(libavg.utils.getMediaDir(pkgpath, 'fonts'))
            self._parentNode.mediadir = libavg.utils.getMediaDir(pkgpath)

    @classmethod
    def get(cls):
        '''
        Get the Application instance
        
        Note: this class method has to be called from the top-level app class:

        >>> class MyApp(gameapp.GameApp):
        ...  pass
        >>> instance = MyApp.get()
        '''
        return cls.instances.get(cls.__name__, None)
        
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
            kwargs['resolution'] = player.getScreenResolution()

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

        super(GameApp, cls).start(*args, **kwargs)

    def quit(self):
        '''
        Quit the application
        The player is stopped if the application has been started by its own
        (eg: no appchooser)
        '''
        self.leave()
        if self.getStarter() is not None:
            player.get().stop()

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

    def initDatastore(self, tag, initialData=None, validator=lambda ds: True,
            autoCommit=True):
        '''
        Initialize and return a Datastore instance
        
        tag: a string that references the datastore. Such tags are uniques within
            the application's scope and may be retrieved from different parts of
            the game's package with getDatastore() method.
        
        See the documentation on Datastore class for the rest of the parameters.

        >>> myDs = MyApp.get().initDatastore('hiscore', [])
        >>> myDs.data.append(ScoreEntry(12321, 'oxi'))
        >>> myDs.commit()
        '''

        if type(tag) != str:
            raise TypeError('Tag must be a string instead of %s' % type(tag))

        if tag in self.__datastores:
            raise RuntimeError('Datastore %s already initialized')
        
        ds = Datastore(self.getUserdataPath(tag + '.pkl'), initialData, validator,
                autoCommit)
                
        self.__datastores[tag] = ds
        
        return ds
    
    def getDatastore(self, tag):
        '''
        Return an initialized Datastore instance given its tag
        '''
        return self.__datastores.get(tag, None)
        
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


