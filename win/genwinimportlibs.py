#!/usr/bin/env python

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

