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
import time
import cPickle as pickle

import libavg


class Persist(object):
    def __init__(self, storeFile, initialData, validator=lambda v: True,
            autoCommit=False):
        self.__storeFile = storeFile
        
        if hasattr(initialData, '__call__'):
            initialData = initialData()
        elif initialData is None:
            initialData = dict()

        if os.path.exists(self.__storeFile):
            if not os.path.isfile:
                raise RuntimeError('%s dump file is not a plain file' % self)
            elif not os.access(self.__storeFile, os.R_OK | os.W_OK):
                raise RuntimeError('%s dump file'
                        'cannot be accessed with r/w permissions' % self)

        try:
            f = open(self.__storeFile)
        except IOError:
            libavg.logger.debug('Initializing %s' % self)
            self.data = initialData
            self.commit()
        else:
            try:
                self.data = pickle.load(f)
            except:
                f.close()
                libavg.logger.error('Persist %s is corrupted or unreadable, '
                        'reinitializing' % self)
                self.data = initialData
                self.commit()
            else:
                f.close()
                if not validator(self.data):
                    libavg.logger.error('Sanity check failed for %s, '
                            'reinitializing' % self)
                    self.data = initialData
                    self.commit()
                else:
                    libavg.logger.debug('%s successfully loaded' % self)

        if autoCommit:
            import atexit
            atexit.register(self.commit)

    def __repr__(self):
        return '<%s %s>' % (self.__class__.__name__, self.__storeFile)

    @property
    def storeFile(self):
        return self.__storeFile

    def commit(self):
        tempFile = self.__storeFile + '.tmp.' + str(int(time.time() * 1000))

        try:
            with open(tempFile, 'wb') as f:
                pickle.dump(self.data, f)
        except Exception, e:
            libavg.logger.error('Cannot save %s (%s)' % (self.__storeFile, str(e)))
            return False
        else:
            if os.path.exists(self.__storeFile):
                try:
                    os.remove(self.__storeFile)
                except Exception, e:
                    libavg.logger.error('Cannot overwrite dump file '
                            '%s (%s)' % (self, str(e)))
                    return False
            try:
                os.rename(tempFile, self.__storeFile)
            except Exception, e:
                libavg.logger.error('Cannot save %s (%s)' % (self, str(e)))
                os.remove(tempFile)
                return False
            else:
                libavg.logger.debug('%s saved' % self)
                return True


class UserPersistentData(Persist):
    def __init__(self, appName, fileName, *args, **kargs):
        basePath = os.path.join(self._getUserDataPath(), appName)
        fullPath = os.path.join(basePath, '%s.pkl' % fileName)

        try:
            os.makedirs(basePath)
        except OSError, e:
            import errno
            if e.errno != errno.EEXIST:
                raise

        super(UserPersistentData, self).__init__(fullPath, *args, **kargs)

    def _getUserDataPath(self):
        if os.name == 'posix':
            path = os.path.join(os.environ['HOME'], '.avg')
        elif os.name == 'nt':
            path = os.path.join(os.environ['APPDATA'], 'Avg')
        else:
            raise RuntimeError('Unsupported system %s' % os.name)

        return path


if __name__ == '__main__':
    testFile = './testfile.pkl'
    initialData = {'initial': True}
    p = Persist(testFile, initialData)
    p.commit()
    p.data['initial'] = False
    p.commit()

    p = Persist(testFile, initialData)
    print not p.data['initial']

    os.unlink(testFile)

    p = UserPersistentData('myapp', 'hiscore', initialData)
    p.data['initial'] = False
    p.commit()

    print p

