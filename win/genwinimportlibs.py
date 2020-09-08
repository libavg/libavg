#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2011-2020 Ulrich von Zadow
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

import os
import string

os.chdir("deps")

def tmpFileToDef(baseName):
    tmpFile = open("tmp", "r")
    defFile = open(baseName+".def", "w")
    defFile.write("EXPORTS\n")
    content = tmpFile.readlines()
    i = 0
    while content[i].find("ordinal hint") == -1:
        i += 1
    isEmptyLine = False;
    i += 2

    while not(isEmptyLine):
        line = string.split(content[i])
        isEmptyLine = len(line) == 0
        if not(isEmptyLine):
            defFile.write(line[3]+"\n")
        i += 1
    defFile.close()


for dllName in os.listdir("bin"):
    if dllName[-4:] == ".dll":
        print "Generating lib for '" + dllName + "'."
        os.system("dumpbin /EXPORTS bin\\"+dllName+" > tmp")
        baseName = dllName[:-4]
        tmpFileToDef(baseName)
        os.system("lib /def:"+baseName+".def /out:lib\\"+baseName+".lib /machine:x86")

