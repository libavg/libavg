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


# TODO: include src/graphics/shaders/*, python/scripts/*
for ext in ("h", "c", "cpp", "py"):
    cmd = 'find . -name "*.'+ext+'" -not -wholename "'+__file__+'"'
    files = os.popen(cmd).readlines()
    for f in files:
        handleFile(f.strip())

print 'changed:', num_changed
print 'skipped:', num_skipped
