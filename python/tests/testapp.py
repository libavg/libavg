#!/usr/bin/env python
# -*- coding: utf-8 -*-
# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
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

import unittest

import optparse
import os
import sys

from libavg import avg, player
import testcase


class TestApp(object):
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
        player.keepWindowOpen()
            
    def getSuiteFactory(self, name):
        return self.__registerdSuiteFactoriesDict[name]
        
    def isSuiteFactoryRegistered(self, name):
        return name in self.__registeredSuiteFactories
    
    def getSuiteFactoryNames(self):
        return list(self.__registeredSuiteFactories)
    
    def getSuiteFactories(self):
        return [ self.__registerdSuiteFactoriesDict[name] 
                for name in self.__registeredSuiteFactories ]

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
        player.loadFile("image.avg")
    
    def __run(self):
        testRunner = unittest.TextTestRunner(verbosity = 2)
        testcase.AVGTestCase.cleanResultDir()
        testResult = testRunner.run(self.__testSuite)
       
        numSkipped = 0
        for suite in self.__testSuite:
            for test in suite:
                if test.skipped():
                    numSkipped += 1
        if numSkipped > 0:
            sys.stderr.write("Skipped "+str(numSkipped)+" tests:\n")
            for suite in self.__testSuite:
                for test in suite:
                    if test.skipped():
                        print "  " + str(test) + ": " + test.skipReason()

        if testResult.wasSuccessful():
            self.__exitOk = TestApp.EXIT_OK

    def __setupTestApp(self):
        self.__setupCommandlineParser()
        self.__parseCommandline()
        self.__setupGlobalPlayerOptions()
        self.__dumpConfig()
        self.__populateTestSuite()

    def __setupGlobalPlayerOptions(self):
        if self.__commandlineOptions.shaderusage == "FULL":
            shaderUsage = avg.SHADERUSAGE_FULL
        elif self.__commandlineOptions.shaderusage == "MINIMAL":
            shaderUsage = avg.SHADERUSAGE_MINIMAL
        elif self.__commandlineOptions.shaderusage == "AUTO":
            shaderUsage = avg.SHADERUSAGE_AUTO
        else:
            sys.stderr.write("\nUnknown value for --shaderusage command-line parameter.\n")
            self.__optionParser.print_help()
            sys.exit(-1)

        player.setOGLOptions(self.__commandlineOptions.usepow2textures, 
                self.__commandlineOptions.usepixelbuffers, 1, shaderUsage, True)

    def __setupCommandlineParser(self):
        self.__optionParser = optparse.OptionParser(
            usage = '%prog [options] [<suite> [testcase] [testcase] [...]]')

        self.__optionParser.add_option("--usepow2textures", 
                dest = "usepow2textures", 
                action = 'store_true',
                default = False, 
                help = "Use power of 2 textures")
        
        self.__optionParser.add_option("--nopixelbuffers", 
                dest = "usepixelbuffers",
                action = 'store_false',
                default = True, 
                help = "Use pixel buffers")

        self.__optionParser.add_option("--shaderusage",
                dest = "shaderusage",
                default = "AUTO", 
                help = "Configure usage of shaders. Valid values are FULL, MINIMAL, and AUTO.")
        
    def __parseCommandline(self):
        self.__commandlineOptions, args = self.__optionParser.parse_args()

        # MFX 2013-11-10: cleanup argv consuming testapp args to avoid clashes
        # with libavg.app.App ArgvExtender
        sys.argv = [sys.argv[0]]

        if len(args): # suite
            suiteFactory = args.pop(0)
            if not(self.isSuiteFactoryRegistered(suiteFactory)):
                sys.stderr.write("Unknown test suite, registered suites:\n")
                for factory in self.getSuiteFactoryNames():
                    sys.stderr.write(factory+"\n")
                sys.stderr.write("\n")
                self.__optionParser.print_usage()

            self.__suitesToRun.append(self.getSuiteFactory(suiteFactory))
            self.__suitesTestSubsets = args
                
        else:
            self.__suitesToRun = self.getSuiteFactories()
        
    def __populateTestSuite(self):
        for suite in self.__suitesToRun:
            self.__testSuite.addTest(suite(self.__suitesTestSubsets))
        
    def __dumpConfig(self):
        player.enableGLErrorChecks(True)
        cats = avg.logger.getCategories()
        for cat in [avg.logger.Category.APP, avg.logger.Category.CONFIG,
                avg.logger.Category.DEPREC]:
            avg.logger.configureCategory(cat, avg.logger.Severity.INFO)
        player.loadString("""
                <avg id="avg" width="160" height="120">
                </avg>
                """)
        player.setTimeout(0, player.stop)
        player.setFramerate(10000)
        player.play()
        for cat, severity in cats.iteritems():
            avg.logger.configureCategory(cat, severity)
