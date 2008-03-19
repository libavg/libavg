from distutils.core import setup, Extension
import os

DEVEL_ROOT='../../../..'

def gatherFilesInDir(dirName):
    return [dirName+fname for fname in os.listdir(dirName) if 
        fname[0] != '.']

# Gather dlls:
dlls=[DEVEL_ROOT+'/bin/'+dllname for dllname in os.listdir(DEVEL_ROOT+'/bin/') 
        if dllname[-4:] == ".dll"]
dlls.append(DEVEL_ROOT+'/Archimedes/libavg_win/python/Debug/avg.pyd')

fontconfig_files=[DEVEL_ROOT+'/etc/fonts/fonts.conf',
                  DEVEL_ROOT+'/etc/fonts/fonts.dtd']
fontconfig_confd_files=gatherFilesInDir(DEVEL_ROOT+"/etc/fonts/conf.d/")

test_files=[fname for fname in gatherFilesInDir('../test/')
        if '.' in fname[3:] ]
test_baseline_files=gatherFilesInDir('../test/baseline/')
test_testmediadir_files=gatherFilesInDir('../test/testmediadir/')

msvc_files=['c:\\Windows\\system32\\'+fname for fname in 
        ['MSVCP71.dll', 'MSVCP71D.dll', 'MSVCR71D.dll']]

magick_files=[DEVEL_ROOT+'/bin/'+fname for fname in os.listdir(DEVEL_ROOT+'/bin/') 
        if fname[-4:] == ".mgk"]

setup(name='libavg',
      version='0.7.1',
      author='Ulrich von Zadow',
      author_email='uzadow@libavg.de',
      url='http://www.libavg.de',
      packages=['libavg'],
      package_dir = {'libavg': '.'},
      data_files=[('Lib/site-packages/libavg', dlls),
                  ('Lib/site-packages/libavg/etc', ['../avgrc']),
                  ('Lib/site-packages/libavg/etc/fonts', fontconfig_files),
                  ('Lib/site-packages/libavg/etc/fonts/conf.d', 
                        fontconfig_confd_files),
                  ('Lib/site-packages/libavg/test', test_files),
                  ('Lib/site-packages/libavg/test/baseline', test_baseline_files),
                  ('Lib/site-packages/libavg/test/testmediadir', 
                        test_testmediadir_files),
                  ('Lib/site-packages/libavg', msvc_files),
                  ('Lib/site-packages/libavg/magick', magick_files)
                 ]
      )


