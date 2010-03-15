#!/usr/bin/env python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2008 Ulrich von Zadow
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

'''
Entry point for libavg unit tests
On autotools-based systems, tests are performed on a local libavg package.
This package is created by copying all the relevant files in a local, temporary
directory, and its scope is added in order to let python to find it.
On windows instead, tests are carried on after distutils takes care of a
system-wide installation.
'''
import unittest

import optparse
import os

import libavg
import testcase


BASELINE_DIR = "baseline"
RESULT_DIR = "resultimages"


def rmBrokenDir():
    try:
        files = os.listdir(RESULT_DIR)
        for file in files:
            os.remove(RESULT_DIR+"/"+file)
    except OSError:
        try:
            os.mkdir(RESULT_DIR)
        except OSError:
            pass


testcase.AVGTestCase.setImageResultDirectory(RESULT_DIR)
testcase.AVGTestCase.setBaselineImageDirectory(BASELINE_DIR)


class TestApp:
    EXIT_OK = 0
    EXIT_FAILURE = 1
        
    def __init__(self):
        self.__exitOk = TestApp.EXIT_FAILURE
        
        self.__registeredSuiteFactories = []
        self.__registerdSuiteFactoriesDict = {}
        
        self.__suitesToRun = []
        self.__suitesTestSubsets = []
        
        self.__testSuite = unittest.TestSuite()
        self.__optionParser = None
        self.__commandlineOptions = None
        self.__player = libavg.avg.Player.get()
            
    def getSuiteFactory(self, name):
        return self.__registerdSuiteFactoriesDict[name]
        
    def isSuiteFactoryRegistered(self, name):
        return name in self.__registeredSuiteFactories
    
    def getSuiteFactoryNames(self):
        return list(self.__registeredSuiteFactories)
    
    def getSuiteFactories(self):
        return [ self.__registerdSuiteFactoriesDict[name] for name in self.__registeredSuiteFactories ]

    def registerSuiteFactory(self, name, suite):
        self.__registeredSuiteFactories.append(name)
        self.__registerdSuiteFactoriesDict[name] = suite
    
    def run(self):
        hasAVGConsoleTest = os.getenv("AVG_CONSOLE_TEST") 
        if hasAVGConsoleTest:
            self.__runVideoTest()
        else:
            self.__setupTestApp()
            self.__run()
    
    def exitCode(self):
        return self.__exitOk
    
    def __iter__(self):
        for name in self.__registeredSuiteFactories:
            yield self.__RegisterdSuitesDict[name]
    
    def __runVideoTest(self):
        self.__player.loadFile("video.avg")
    
    def __run(self):
        testRunner = unittest.TextTestRunner(verbosity = 2)
        rmBrokenDir()
        testResult = testRunner.run(self.__testSuite)
        
        if testResult.wasSuccessful():
            self.__exitOk = TestApp.EXIT_OK

    def __setupTestApp(self):
        self.__setupCommandlineParser()
        self.__parseCommandline()
        self.__setupGlobalPlayerOptions()
        self.__dumpConfig()
        self.__populateTestSuite()

    def __setupGlobalPlayerOptions(self):
        usePow2Textures = self.__commandlineOptions.usepow2textures
        useShaders =  self.__commandlineOptions.usepow2textures
        usePixelBuffer = self.__commandlineOptions.usepow2textures   
        
        if  usePow2Textures or useShaders or usePixelBuffer:
            self.__player.setOGLOptions(usePow2Textures, 
                                        useShaders, 
                                        usePixelBuffer, 1)
        
    def __setupCommandlineParser(self):
        self.__optionParser = optparse.OptionParser(
            usage = '%prog [options] [<suite> [testcase] [testcase] [...]]')
        
        self.__optionParser.add_option("--usepow2textures", 
                                       dest = "usepow2textures", 
                                       action = 'store_true',
                                       default = False, 
                                       help = "Use power of 2 textures")
        
        self.__optionParser.add_option("--useshaders", 
                                       dest = "useshaders",
                                       action = 'store_true',
                                       default = True, 
                                       help = "Use shaders")
                
        self.__optionParser.add_option("--usepixelbuffers", 
                                       dest = "usepixelbuffers",
                                       action = 'store_true',
                                       default = False, 
                                       help = "Use pixel buffers")
        
    def __parseCommandline(self):
        self.__commandlineOptions, args = self.__optionParser.parse_args()
        
        if len(args): # suite
            suiteFactory = args.pop(0)
            if not(self.isSuiteFactoryRegistered(suiteFactory)):
                print "Unknown test suite, registered suites:"
                for factory in self.getSuiteFactoryNames():
                    print factory
                print ''
                self.__optionParser.print_usage()

            self.__suitesToRun.append(self.getSuiteFactory(suiteFactory))
            self.__suitesTestSubsets = args
                
        else:
            self.__suitesToRun = self.getSuiteFactories()
        
    def __populateTestSuite(self):
        for suite in self.__suitesToRun:
            self.__testSuite.addTest(suite(self.__suitesTestSubsets))
        
    def __dumpConfig(self):
        log = libavg.avg.Logger.get()
        log.pushCategories()
        log.setCategories(log.APP | log.WARNING | log.CONFIG  | 0)
        self.__player.loadString("""
                <?xml version="1.0"?>
                <avg id="avg" width="160" height="120">
                </avg>
                """)
        self.__player.setTimeout(0, self.__player.stop)
        self.__player.play()
        log.popCategories()
