from distutils.core import setup, Extension
import os

DEVEL_ROOT='../../../..'

# Gather dlls:
dlls=[DEVEL_ROOT+'/bin/'+dllname for dllname in os.listdir(DEVEL_ROOT+'/bin/') 
        if dllname[-4:] == ".dll"]
dlls.append(DEVEL_ROOT+'/Archimedes/libavg_win/python/Debug/avg.pyd')

fontconfig_files=[DEVEL_ROOT+'/etc/fonts/fonts.conf',
                  DEVEL_ROOT+'/etc/fonts/fonts.dtd']
fontconfig_confd_files=os.listdir(DEVEL_ROOT+"/etc/fonts/conf.d")
fontconfig_confd_files=[DEVEL_ROOT+'/etc/fonts/conf.d/'+fname for 
        fname in fontconfig_confd_files]

test_files=['../test/'+fname for fname in os.listdir('../test')
        if '.' in fname[1:] ]
test_baseline_files=['../test/baseline/'+fname for fname in 
        os.listdir('../test/baseline') if fname[0] != '.']
test_testmediadir_files=['../test/testmediadir/'+fname for fname in 
        os.listdir('../test/testmediadir') if fname[0] != '.']

msvc_files=['c:\\Windows\\system32\\'+fname for fname in 
        ['MSVCP71.dll', 'MSVCP71D.dll', 'MSVCR71D.dll']]


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
                  ('Lib/site-packages/libavg', msvc_files)
                 ]
      )


