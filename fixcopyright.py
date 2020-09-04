#!/usr/bin/env python

import os
import re
import time

TEMPLATE = 'Copyright (C) %s-%s Ulrich von Zadow\n'

num_changed = 0
num_skipped = 0


def handleFile(path):
    global num_changed, num_skipped

    lines = file(path).readlines()
    found = False
    for i, line in enumerate(lines):
        match = re.match(r'((//)|(#))\s*Copyright \(C\) (?P<from>\d{4})(.*) Ulrich von Zadow\s*', line)
        if match:
            year_from = match.group('from')
            year_to = str(time.localtime().tm_year)
            if path[-2:] == 'py':
                lines[i] = '# ' + TEMPLATE % (year_from, year_to)
            else:
                lines[i] = '//  ' + TEMPLATE % (year_from, year_to)
            found = True
            break
    if found:
        num_changed +=1
        outFile = open(path, "w")
        for line in lines:
            outFile.write(line)
    else:
        num_skipped +=1
        print 'skipping', path


for ext in ("h", "c", "cpp", "py"):
    cmd = 'find . -name "*.'+ext+'" -not -wholename "'+__file__+'"'
    files = os.popen(cmd).readlines()
    for f in files:
        handleFile(f.strip())

print 'changed:', num_changed
print 'skipped:', num_skipped
