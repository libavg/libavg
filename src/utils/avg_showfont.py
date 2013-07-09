#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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

from libavg import avg, app


class ShowFont(app.MainDiv):
    def onArgvParserCreated(self, parser):
        self._usage = 'Usage: %s [<fontname> [<text>]] [options]\n\n' \
                '  Shows all available variants of a font.\n' \
                '  If <text> is given, displays the text.\n' \
                '  If <fontname> is not given, dumps a list of all fonts available.' \
                %parser.get_prog_name()
        parser.set_usage(self._usage)

    def onArgvParsed(self, options, args, parser):
        if len(args) == 0:
            fontList = avg.WordsNode.getFontFamilies()
            print 'Available fonts:'
            print fontList
            print
            print self._usage
            print '  Option -h or --help gives a full help.'
            exit()

        self._fontname = args[0]
        if len(args) > 1:
            self._displayText = args[1]
        else:
            self._displayText = None

    def onInit(self):
        variants = avg.WordsNode.getFontVariants(self._fontname)
        print variants

        y = 10
        for variant in variants:
            if self._displayText:
                text = self._displayText
            else:
                text = self._fontname + ": " + variant
            avg.WordsNode(text=text, font=self._fontname, variant=variant, fontsize=24,
                    pos=(10, y), parent=self)
            y += 50


if __name__ == '__main__':
    app.App().run(ShowFont())

