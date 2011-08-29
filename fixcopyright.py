#!/usr/bin/env python

import os
import re

def handleFile(path):
    lines = file(path).readlines()
    lineNumber = 0
    found = False
    for i, line in enumerate(lines):
        match = re.match(r'((//)|(#))\s*Copyright \(C\) 2003-(.*) Ulrich von Zadow\s*', line)
#        m = re.match(r'#include\s*["<]([\-_a-zA-Z0-9\.\\/]+)[">]\s*', l)
        if match:
            if path[-2:] == 'py':
                lines[i] = "# Copyright (C) 2003-2011 Ulrich von Zadow\n"
            else:
                lines[i] = "//  Copyright (C) 2003-2011 Ulrich von Zadow\n"
            found = True
    if not(found):
        print path


for ext in ("h", "c", "cpp", "py"):
    cmd = 'find . -name "*.'+ext+'"'
    files = os.popen(cmd).readlines()
    for f in files:
        handleFile(f.strip())

