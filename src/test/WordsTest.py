#!/usr/bin/python
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

import unittest

import sys, time, os, platform

# Import the correct version of libavg. Since it should be possible to
# run the tests without installing libavg, we add the location of the 
# uninstalled libavg to the path.
# TODO: This is a mess. 
sys.path += ['../wrapper/.libs', '../python']
if platform.system() == 'Darwin':
    sys.path += ['../..']     # Location of libavg in a mac installation. 

if platform.system() == 'Windows':
    from libavg import avg    # Under windows, there is no uninstalled version.
else:    
    import avg

from testcase import *

class WordsTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName, 24)
    
    def testSimpleWords(self):
        def checkFont():
            node = Player.getElementByID("sanstext")
            self.assert_(node.variant=="bold")
    
        def checkUnicodeText():
            node = Player.getElementByID("sanstext")
            node.text = u"föa"
        
        fontList = avg.Words.getFontFamilies()
        try:
            fontList.index("Bitstream Vera Sans")
        except ValueError:
            self.assert_(False)
        variantList = avg.Words.getFontVariants("Bitstream Vera Sans")
        self.assert_(len(variantList) >= 4)
        Player.loadString("""
          <avg width="160" height="120">
            <words x="1" y="1" fontsize="12" font="Bitstream Vera Sans" 
                text="Bitstream Vera Sans" variant="roman"/>
            <words id="sanstext" x="1" y="16" fontsize="12" font="Bitstream Vera Sans" 
                variant="bold" text="Bold"/>
          </avg>
        """)
        pos = Player.getElementByID("sanstext").getGlyphPos(0)
        self.start(None,
                (lambda: self.compareImage("testSimpleWords", True),
                 checkFont,
                 checkUnicodeText
                ))

    def testGlyphPos(self):
        def posAlmostEqual(pos1, pos2):
            return math.fabs(pos1[0]-pos2[0]) <= 2 and math.fabs(pos1[1]-pos2[1]) <= 2
        
        node = Player.createNode("words", {"text":"Bold", "font":"Bitstream Vera Sans"})
        self.assert_(node.getGlyphPos(0) == (0,0))
        size = node.getGlyphSize(0)
        self.assert_(posAlmostEqual(size, (10, 18)))
        self.assert_(posAlmostEqual(node.getGlyphPos(3), (22,0)))
        size = node.getGlyphSize(3)
        self.assert_(posAlmostEqual(size, (8, 18)))
        self.assertException(lambda: node.getGlyphPos(4))
        node.text=u"föa"
        self.assert_(posAlmostEqual(node.getGlyphPos(1), (4,0)))
#        print [ node.getGlyphPos(i) for i in range(3)]
        self.assert_(posAlmostEqual(node.getGlyphPos(2), (12,0)))
        self.assertException(lambda: node.getGlyphPos(3))

    def testParaWords(self):
        Player.loadString("""
          <?xml version="1.0"?>
          <avg width="160" height="120">
              <line pos1="(0.5, 0)" pos2="(0.5, 50)" color="FF0000"/>
              <line pos1="(119.5, 0.5)" pos2="(119.5, 50)" color="FF0000"/>
              <line pos1="(74.5, 60)" pos2="(74.5, 110)" color="FF0000"/>
              <words id="para" x="1" y="1" fontsize="12" width="70" 
                      font="Bitstream Vera Sans"
                      text="Left-justified paragraph."/>
              <words id="paracenter" x="120" y="1" fontsize="12" width="70" 
                      font="Bitstream Vera Sans" alignment="center"
                      text="Centered paragraph"/>
              <words id="pararight" x="75" y="60" fontsize="12" width="70" 
                      font="Bitstream Vera Sans" alignment="right">
                      Right-justified paragraph.<i>l</i></words>
              <words id="paralinespacing" x="80" y="60" fontsize="12" width="70" 
                      font="Bitstream Vera Sans" linespacing="-4"
                      text="Paragraph with custom line spacing."/>
          </avg>
        """)
        self.start(None,
                [lambda: self.compareImage("testParaWords", True)])

    def testJustify(self):
        Player.loadString("""
          <avg width="160" height="120">
            <words x="1" y="1" fontsize="12" font="Bitstream Vera Sans"
                variant="roman" justify="true" width="100"
                text="Justified paragraph more than one line long."/>
          </avg>
        """)
        self.start(None,
                [lambda: self.compareImage("testJustify", True)])

    def testWrapMode(self):
        def setCharMode():
            node = Player.getElementByID("words")
            node.wrapmode = 'char'
        
        def setWordMode():
            node = Player.getElementByID("words")
            node.wrapmode = 'word'
        
        def setWordCharMode():
            node = Player.getElementByID("words")
            node.wrapmode = 'wordchar'
            
        Player.loadString("""
        <avg width="160" height="120">
          <words x="1" y="1" fontsize="12" font="Bitstream Vera Sans"
              variant="roman" width="100" id="words"
              text="Wrapped paragraph more than one line long.
                Withaverylongpackedlinewithnobreaks"/>
        </avg>
        """)
        self.start(None,
            [lambda: self.compareImage("testWrapMode1", True),
             setCharMode,
             lambda: self.compareImage("testWrapMode2", True),
             setWordMode,
             lambda: self.compareImage("testWrapMode3", True),
             setWordCharMode,
             lambda: self.compareImage("testWrapMode4", True),
             ])

    def testWordsMask(self):
        def setMask():
            try:
                node.maskhref = "mask.png"
            except RuntimeError:
                print "Skipping testWordsMask - no shader support."
                Player.stop()
           
        def setColor():
            node.color = "FFFF00"

        def setOpacity():
            node.opacity = 0.5

        def setPos():
            node.maskpos = (-20, 0)
            node.opacity = 1 

        Player.loadString("""
        <avg width="160" height="120">
          <words x="1" y="1" fontsize="12" font="Bitstream Vera Sans"
              variant="roman" width="100" id="words"
              text="Wrapped paragraph more than one line long."/>
        </avg>
        """)
        node = Player.getElementByID("words")
        self.start(None,
                (setMask,
                 lambda: self.compareImage("testWordsMask1", False),
                 setColor,
                 lambda: self.compareImage("testWordsMask2", False),
                 setOpacity,
                 lambda: self.compareImage("testWordsMask3", False),
                 setPos,
                 lambda: self.compareImage("testWordsMask4", False),
                ))

    def testHinting(self):
        def checkPositions():
            root = Player.getRootNode()
#            node0 = root.getChild(0)
#            for i in range(len(node0.text)):
#                print node0.getGlyphPos(i)
            self.assert_(root.getChild(0).getGlyphPos(6) != 
                root.getChild(1).getGlyphPos(6))

        if platform.system() == "Linux":
            print "Skipping testHinting - Linux support requires modified font config."
        else:
            Player.loadString("""
            <avg width="160" height="120">
              <words x="1" y="1" fontsize="12" font="Bitstream Vera Sans"
                  variant="roman" hint="false"
                  text="Lorem ipsum dolor (no hinting)"/>
              <words x="1" y="15" fontsize="12" font="Bitstream Vera Sans"
                  variant="roman" hint="true"
                  text="Lorem ipsum dolor (hinting)"/>
            </avg>
            """)
            self.start(None,
                [checkPositions
                ])


    def testSpanWords(self):
        def setTextAttrib():
            self.baselineBmp = Player.screenshot()
            node = Player.getElementByID("words")
            node.text = self.text
        
        def checkSameImage():
            bmp = Player.screenshot()
            self.assert_(self.areSimilarBmps(bmp, self.baselineBmp, 0, 0))
        
        def createUsingDict():
            Player.getElementByID("words").unlink()
            node = Player.createNode("words", {
                    "id":"words", "x":1, "y":1, "fontsize":12, "width":120,
                    "font":"Bitstream Vera Sans", "variant": "roman",
                    "text":self.text
                })
            Player.getRootNode().appendChild(node)
        
        self.text = """
              Markup: 
              <span size='14000' rise='5000' foreground='red'>span</span>, 
              <i>italics</i>, <b>bold</b>
        """
        Player.loadString("""
          <avg width="160" height="120">
            <words id="words" x="1" y="1" fontsize="12" width="120" 
                font="Bitstream Vera Sans" variant="roman">
        """
        +self.text+
        """
            </words>
          </avg>
        """)
        self.start(None,
                [lambda: self.compareImage("testSpanWords", True),
                 setTextAttrib,
                 lambda: self.compareImage("testSpanWords", True),
                 checkSameImage,
                 createUsingDict,
                 lambda: self.compareImage("testSpanWords", True),
                 checkSameImage,
                ])
    
    def testDynamicWords(self):
        def changeText():
            node = Player.getElementByID("dynamictext")
            oldwidth = node.width
            node.text = "blue" 
            self.assert_(node.width != oldwidth)
            node.color = "404080"
            node.x += 10
        
        def changeHeight():
            node = Player.getElementByID("dynamictext")
            node.height = 28
        
        def activateText():
            Player.getElementByID('dynamictext').active = 1
        
        def deactivateText():
            Player.getElementByID('dynamictext').active = 0
        
        def changeFont():
            node = Player.getElementByID("dynamictext")
            node.font = "Bitstream Vera Sans"
            node.height = 0
            node.fontsize = 30
        
        def changeFont2():
            node = Player.getElementByID("dynamictext")
            node.fontsize = 18
        
        def changeTextWithInvalidTag():
            node = Player.getElementByID("dynamictext")
            try:
                node.text = "This <invalid_tag/>bombs"
            except:
                node.text = "except"
            self.assert_(node.text == "except")
        
        self.start("dynamictext.avg",
                (lambda: self.compareImage("testDynamicWords1", True),
                 changeText,
                 changeHeight,
                 changeFont,
                 lambda: self.compareImage("testDynamicWords2", True),
                 deactivateText,
                 lambda: self.compareImage("testDynamicWords3", True),
                 activateText,
                 changeFont2,
                 lambda: self.compareImage("testDynamicWords4", True),
                 changeTextWithInvalidTag
                ))

    def testI18NWords(self):
        def changeUnicodeText():
            Player.getElementByID("dynamictext").text = "Arabic nonsense: ﯿﭗ"
        
        self.start("i18ntext.avg",
                (lambda: self.compareImage("testI18NWords1", True),
                 changeUnicodeText,
                 lambda: self.compareImage("testI18NWords2", True)
                ))

    def testRawText(self):
        def createDynNodes():
            self.dictdnode = Player.createNode("words", 
                    {'text':'&lt;test dyndict&amp;', 
                     'rawtextmode':True, 
                     'x':1, 
                     'y':65, 
                     'font':'Bitstream Vera Sans', 
                     'variant': 'roman', 
                     'fontsize':12})
            Player.getRootNode().appendChild(self.dictdnode)

            self.xmldnode = Player.createNode("""
                <words text="&lt;test dynattr&amp;" fontsize="12" 
                        font="Bitstream Vera Sans" variant="roman" rawtextmode="true"
                        x="1" y="85"/>""")
            Player.getRootNode().appendChild(self.xmldnode)
        
        def switchRawMode():
            self.dictdnode.rawtextmode = False
            Player.getElementByID('nodeval').rawtextmode = True
            Player.getElementByID('attrib').rawtextmode = True
        
        def bombIt():
            def cantRun():
                self.xmldnode.rawtextmode = False
                self.assert_(0)
            self.assertException(cantRun)
        
        def assignNewTexts():
            text = u'&ùùààxx>'
            self.dictdnode.rawtextmode = True
            self.dictdnode.text = text
            self.xmldnode.text = text
            Player.getElementByID('nodeval').text = text
            Player.getElementByID('attrib').text = text
        
        self.start("rawtext.avg",
                (lambda: self.compareImage("testRawText1", True),
                 createDynNodes,
                 lambda: self.compareImage("testRawText2", True),
                 switchRawMode,
                 lambda: self.compareImage("testRawText3", True),
                 bombIt,
                 assignNewTexts,
                 lambda: self.compareImage("testRawText4", True),
                ))

    def testWordsBR(self):
        Player.loadString("""
          <avg width="160" height="120">
            <words id="words" x="1" y="1" fontsize="12" 
                font="Bitstream Vera Sans" variant="roman">
               paragraph 1<br/>paragraph 2
            </words>
          </avg>
        """)
        self.start(None,
                [lambda: self.compareImage("testWordsBR", True)
                ])

    def testLetterSpacing(self):
        def setSpacing():
            Player.getElementByID("words1").letterspacing=-2
            Player.getElementByID("words2").letterspacing=-2
        
        Player.loadString("""
          <avg width="160" height="120">
            <words id="words1" x="1" y="1" fontsize="12" font="Bitstream Vera Sans" 
                    variant="roman">
               normal
               <span letter_spacing="-2048"> packed</span>
               <span letter_spacing="2048"> spaced</span>
            </words>
            <words id="words2" x="1" y="20" fontsize="12" font="Bitstream Vera Sans"
                    variant="roman" letterspacing="2" text="spaced"/>
          </avg>
        """)
        self.start(None,
                (lambda: self.compareImage("testLetterSpacing1", True),
                 setSpacing,
                 lambda: self.compareImage("testLetterSpacing2", True)
                ))
       
    def testPositioning(self):
        def click(pos):
            helper.fakeMouseEvent(avg.CURSORDOWN, True, False, False,
                        int(pos[0]), int(pos[1]), 1)
           
        def testInside(bInside):
            ok = bInside == self.clicked
            self.clicked = False
            return ok

        def onMouse(event):
            self.clicked = True

        Player.loadString("""
          <avg width="160" height="120">
            <line pos1="(4, 20.5)" pos2="(157, 20.5)" color="FF0000"/>
            <line pos1="(4.5, 20.5)" pos2="(4.5, 110)" color="FF0000"/>
            <line pos1="(156.5, 20.5)" pos2="(156.5, 110)" color="FF0000"/>
            <line pos1="(80.5, 20.5)" pos2="(80.5, 110)" color="FF0000"/>
            <words id="left" x="4" y="20" fontsize="12" font="Bitstream Vera Sans"
                    variant="roman" text="Norm"/>
            <words x="45" y="20" fontsize="12" font="Bitstream Vera Sans"
                    variant="roman" text="orm"/>
            <words x="75" y="20" fontsize="12" font="Bitstream Vera Sans"
                    variant="roman" text="ÖÄÜ"/>
            <words x="4" y="40" fontsize="12" font="Bitstream Vera Sans"
                    variant="oblique" text="Jtalic"/>
            <words id="right" x="156" y="60" fontsize="12" alignment="right" 
                    font="Bitstream Vera Sans" variant="roman" text="Right-aligned"/>
            <words id="center" x="80" y="80" fontsize="12" alignment="center" 
                    font="Bitstream Vera Sans" variant="roman" text="Centered"/>
          </avg>
        """)
        for id in ["left", "center", "right"]:
            Player.getElementByID(id).setEventHandler(avg.CURSORDOWN, avg.MOUSE,
                    onMouse)
        self.clicked = False
        helper = Player.getTestHelper()
        leftWidth = Player.getElementByID("left").getMediaSize()[0]
        centerWidth = Player.getElementByID("center").getMediaSize()[0]
        rightWidth = Player.getElementByID("right").getMediaSize()[0]

        self.start(None,
                (lambda: self.compareImage("testPositioning", True),
                 lambda: click((4,20)),
                 lambda: self.assert_(testInside(True)),
                 lambda: click((3,20)),
                 lambda: self.assert_(testInside(False)),
                 lambda: click((3+leftWidth,20)),
                 lambda: self.assert_(testInside(True)),
                 lambda: click((4+leftWidth,20)),
                 lambda: self.assert_(testInside(False)),
                
                 lambda: click((81-centerWidth/2,80)),
                 lambda: self.assert_(testInside(True)),
                 lambda: click((80-centerWidth/2,80)),
                 lambda: self.assert_(testInside(False)),
                 lambda: click((80+centerWidth/2,80)),
                 lambda: self.assert_(testInside(True)),
                 lambda: click((81+centerWidth/2,80)),
                 lambda: self.assert_(testInside(False)),

                 lambda: click((156-rightWidth,60)),
                 lambda: self.assert_(testInside(True)),
                 lambda: click((155-rightWidth,60)),
                 lambda: self.assert_(testInside(False)),
                 lambda: click((155,60)),
                 lambda: self.assert_(testInside(True)),
                 lambda: click((156,60)),
                 lambda: self.assert_(testInside(False)),
                ))

    def testInvalidColor(self):
        def testColor(color):
            Player.createNode('words', {'color':color})
        
        def assignValidColor():
            testColor('123456')
        
        def assignInvalidColor1():
            testColor('1234567')
        
        def assignInvalidColor2():
            testColor('xxx')
        
        def assignInvalidColor3():
            testColor('xxxxxx')

        Player.loadString("""
          <avg width="160" height="120">
          </avg>
        """)
        self.start(None, (
            self.assertException(assignInvalidColor1),
            self.assertException(assignInvalidColor2),
            self.assertException(assignInvalidColor3),
                    ))

    def testFontDir(self):
        avg.Words.addFontDir('extrafonts')
        Player.loadString("""
          <avg width="160" height="120">
            <words font="testaddfontdir" fontsize="50" text="ABAAA"/>
          </avg>
        """)
        self.start(None,
                (lambda: self.compareImage("testFontDir", True),
                ))

def wordsTestSuite(tests):
    availableTests = (
            "testSimpleWords",
            "testGlyphPos",
            "testParaWords",
            "testJustify",
            "testWrapMode",
            "testWordsMask",
            "testHinting",
            "testSpanWords",
            "testDynamicWords",
            "testI18NWords",
            "testRawText",
            "testWordsBR",
            "testLetterSpacing",
            "testPositioning",
            "testInvalidColor",
            "testFontDir",
            )
    return AVGTestSuite (availableTests, WordsTestCase, tests)

Player = avg.Player.get()
if __name__ == '__main__':
    runStandaloneTest (wordsTestSuite)


