#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2008 Archimedes Solutions GmbH,
# Saarbrücker Str. 24b, Berlin, Germany
#
# This file contains proprietary source code and confidential
# information. Its contents may not be disclosed or distributed to
# third parties unless prior specific permission by Archimedes
# Solutions GmbH, Berlin, Germany is obtained in writing. This applies
# to copies made in any form and using any medium. It applies to
# partial as well as complete copies.

from distutils.core import setup, Extension
import os, sys, shutil, subprocess, glob

DEVEL_ROOT='../../../'
LIBAVG_SRC_DIR='../../src/'
site_packages_path = sys.exec_prefix+"\\Lib\\site-packages\\libavg"

ERROR_STR= """Error removing %(path)s, %(error)s """

def rmgeneric(path, __func__):
    try:
        __func__(path)
        print 'Removed ', path
    except OSError, (errno, strerror):
        print ERROR_STR % {'path' : path, 'error': strerror }
            
def removeall(path):
    if not os.path.isdir(path):
        return
    files=os.listdir(path)
    for x in files:
        fullpath=os.path.join(path, x)
        if os.path.isfile(fullpath):
            f=os.remove
            rmgeneric(fullpath, f)
        elif os.path.isdir(fullpath):
            removeall(fullpath)
            f=os.rmdir
            rmgeneric(fullpath, f)


def gatherFilesInDir(dirName, exclude=[]):
    return [os.path.join(dirName, fname) for fname in os.listdir(dirName) if
        not os.path.isdir(os.path.join(dirName, fname)) and not fname in exclude]

def gatherPythonFilesInDir(dirName):
    return [dirName + fname for fname in os.listdir(dirName) if
        fname[-3:] == ".py"]

fnull = open(os.devnull, 'w')
try:
    rc = subprocess.call(["svn",], stdout=fnull, stderr=fnull)
    fnull.close()
except WindowsError:
    print "Failed to execute subversion - no release info."
    rc = 1

if rc != 1:
    print 'WARNING: SVN returned a bad code'
    svnError()

# Gather dlls:
dlls=[DEVEL_ROOT+'/bin/'+dllname for dllname in os.listdir(DEVEL_ROOT+'/bin/') 
        if dllname[-4:] == ".dll" or dllname[-9:] == ".manifest"]
dlls.append('../Release/avg.pyd')

test_files=[fname for fname in gatherFilesInDir(LIBAVG_SRC_DIR+'test/')
        if '.' in fname[-4:] ]
test_baseline_files=gatherFilesInDir(LIBAVG_SRC_DIR+'test/baseline/')

test_testmediadir_files=gatherFilesInDir(LIBAVG_SRC_DIR+'test/testmediadir/')
test_fonts_files=gatherFilesInDir(LIBAVG_SRC_DIR+'test/fonts/')

videotest_files = gatherFilesInDir(LIBAVG_SRC_DIR+'video/testfiles')

python_files = gatherPythonFilesInDir(LIBAVG_SRC_DIR+'python/')
python_files += [
        LIBAVG_SRC_DIR+'test/testcase.py',
        LIBAVG_SRC_DIR+'test/testapp.py',
        ]
python_ui_files = gatherPythonFilesInDir(LIBAVG_SRC_DIR+'python/ui/')
python_data_files = gatherFilesInDir(LIBAVG_SRC_DIR+'python/data/',
        ('Makefile.am',))

assets_files=[]

data_files_list=[
        ('Lib/site-packages/libavg', dlls),
        ('Lib/site-packages/libavg', ('version.txt',)),
        ('Lib/site-packages/libavg/test', test_files),
        ('Lib/site-packages/libavg/test/baseline', test_baseline_files),
        ('Lib/site-packages/libavg/test/testmediadir', test_testmediadir_files),
        ('Lib/site-packages/libavg/test/fonts', test_fonts_files),
        ('Lib/site-packages/libavg/video/testfiles', videotest_files),
        ('Lib/site-packages/libavg/plugin', ('../Release/colornode.dll',)),
        ('Lib/site-packages/libavg', python_files),
        ('Lib/site-packages/libavg/ui', python_ui_files),
        ('Lib/site-packages/libavg/data', python_data_files),
        ('Lib/site-packages/libavg', assets_files)
        ]

fontconfig_files=[DEVEL_ROOT+'etc/fonts/fonts.conf',
                  DEVEL_ROOT+'etc/fonts/fonts.dtd']

data_files_list.append(('Lib/site-packages/libavg/etc',
        [LIBAVG_SRC_DIR+'avgrc']))

data_files_list += (
        ('Lib/site-packages/libavg/etc/fonts', fontconfig_files),
    )

shutil.copy(LIBAVG_SRC_DIR+'wrapper/__init__.py', '__init__.py')

revision = 'svn'

try:
    nenv = os.environ
    nenv['LANG'] = 'EN'
    output = subprocess.Popen(["svn", "info", DEVEL_ROOT+'/libavg'],
        stdout=subprocess.PIPE, env=nenv).communicate()[0]
    f = open("version.txt", "w")
    f.write(output)
    f.close()
except:
    pass
    f = open("version.txt", "w")
    f.write("")
    f.close()
else:
    import re
    cre = re.compile('. (\d+)$', re.M)
    match = cre.search(output)
    if match and match.groups:
        revision = 'r' + match.groups()[0]

scripts = glob.glob(os.path.join(LIBAVG_SRC_DIR, 'utils', 'avg_*.py'))
batches = []

f = open('batch_template.txt')
batchTemplate = f.read(1024)
f.close()

for py in map(os.path.basename, scripts):
    batch = os.path.splitext(py)[0] + '.bat'
    fw = open(batch, 'w')
    fw.write(batchTemplate.replace('#$#PYSCRIPT#$#', py))
    fw.close()
    batches.append(batch)
    

setup(name='libavg',
      version='1.6.0.%s' % revision,
      author='Ulrich von Zadow',
      author_email='uzadow@libavg.de',
      url='http://www.libavg.de',
      packages=['libavg'],
      package_dir = {'libavg': '.'},
      data_files = data_files_list,
      scripts=scripts + batches,
      )

for batch in batches:
    os.unlink(batch)
 