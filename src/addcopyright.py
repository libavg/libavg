#!/usr/bin/python

import sys, os, fnmatch

CopyrightNoticeLines = file("Copyright").read().splitlines()

for dir in ["base", "conradrelais", "graphics", "parport", "player", "python"]:
    files = os.listdir(dir)
    for f in files:
        if fnmatch.fnmatch(f, "*.cpp") or fnmatch.fnmatch(f, "*.h"):
            fname = dir+"/"+f
            print "Processing " + fname
            fobj = file(fname, "r+")
            str = fobj.read()
            lines = str.splitlines()
            while lines[0].find('//') == 0:
                lines = lines[1:]
            lines = CopyrightNoticeLines+lines
            fobj.seek(0)
            fobj.truncate()
            for line in lines:
                fobj.write(line+"\n")
