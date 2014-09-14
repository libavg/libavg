#!/usr/bin/python
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

import platform

from libavg import avg, player
from testcase import *

class WordsTestCase(AVGTestCase):
    def __init__(self, testFuncName):
        AVGTestCase.__init__(self, testFuncName)
   
    def testSimpleWords(self):
        def checkFont():
            self.assertEqual(node.variant, "bold")
    
        def checkUnicodeText():
            node.text = u"föa"
            avg.WordsNode(text=u"öäü", font="Bitstream Vera Sans")
       
        fontList = avg.WordsNode.getFontFamilies()
        try:
            fontList.index("Bitstream Vera Sans")
        except ValueError:
            self.fail("Font not found")
        variantList = avg.WordsNode.getFontVariants("Bitstream Vera Sans")
        self.assert_(len(variantList) >= 4)
        root = self.loadEmptyScene()
        avg.WordsNode (pos=(1,1), fontsize=12, font="Bitstream Vera Sans",
                text="Bitstream Vera Sans", variant="roman", parent=root)
        node = avg.WordsNode(pos=(1,16), fontsize=12, font="Bitstream Vera Sans",
                text="Bold", variant="bold", parent=root)
        self.assertNotEqual(node.size, (0,0))
        node.getGlyphPos(0)
        self.start(True, 
                (lambda: self.compareImage("testSimpleWords"),
                 checkFont,
                 checkUnicodeText,
                ))

    def testRedrawOnDemand(self):

        def changeText(newText):
            size = node.size
            node.text = newText 
            self.assertNotEqual(node.size, size)

        def changeFont():
            size = node.size
            node.fontsize = 18
            self.assertNotEqual(node.size, size)

        root = self.loadEmptyScene()
        node = avg.WordsNode(font="Bitstream Vera Sans", fontsize=12, text="foo", 
                parent=root)
        changeText("foobar")
        self.start(True, 
                (lambda: changeText("bar"),
                 changeFont,
                ))

    def testFontStyle(self):

        def setStyle(node, style):
            node.fontstyle = style
            self.assert_(node.fontsize == 15)

        fontStyle = avg.FontStyle(font="Bitstream Vera Sans", variant="Roman",
                fontsize=12)
        self.assert_(fontStyle.font == "Bitstream Vera Sans")
        root = self.loadEmptyScene()
        words = avg.WordsNode(pos=(1,1), fontstyle=fontStyle, text="Bitstream Vera Sans", 
                parent=root)
        avg.WordsNode (pos=(1,16), fontstyle=fontStyle, variant="bold", text="Bold",
                parent=root)
        otherFontStyle = fontStyle
        otherFontStyle.fontsize = 15
        self.start(True, 
                (lambda: self.compareImage("testFontStyle1"),
                 lambda: setStyle(words, otherFontStyle),
                 lambda: self.compareImage("testFontStyle2"),
                ))

    def testBaseStyle(self):
        attrs = {"font": "Bitstream Vera Sans",
                 "variant": "Bold",
                 "color": "FF0000",
                 "aagamma": 0.5,
                 "fontsize": 20,
                 "indent": 1,
                 "linespacing": 2,
                 "alignment": "right",
                 "wrapmode": "char",
                 "justify": True,
                 "letterspacing": 3,
                 "hint": False}
        defaultStyle = avg.FontStyle()
        fontStyle1 = avg.FontStyle(basestyle=defaultStyle, **attrs)
        for attrName in attrs.iterkeys():
            self.assert_(getattr(fontStyle1, attrName) != getattr(defaultStyle, attrName))
            self.assert_(getattr(fontStyle1, attrName) == attrs[attrName])
        fontStyle2 = avg.FontStyle(basestyle=fontStyle1)
        for attrName in attrs.iterkeys():
            self.assert_(getattr(fontStyle2, attrName) == getattr(fontStyle1, attrName))

    def testGlyphPos(self):
        def posAlmostEqual(pos1, pos2):
            return math.fabs(pos1[0]-pos2[0]) <= 2 and math.fabs(pos1[1]-pos2[1]) <= 2
        
        node = avg.WordsNode(text="Bold", font="Bitstream Vera Sans")
        self.assertEqual(node.getGlyphPos(0), (0,0))
        size = node.getGlyphSize(0)
        self.assert_(posAlmostEqual(size, (10, 18)))
        self.assert_(posAlmostEqual(node.getGlyphPos(3), (22,0)))
        size = node.getGlyphSize(3)
        self.assert_(posAlmostEqual(size, (8, 18)))
        self.assertRaises(RuntimeError, lambda: node.getGlyphPos(4))
        node.text=u"föa"
        self.assert_(posAlmostEqual(node.getGlyphPos(1), (4,0)))
#        print [ node.getGlyphPos(i) for i in range(3)]
        self.assert_(posAlmostEqual(node.getGlyphPos(2), (12,0)))
        self.assertRaises(RuntimeError, lambda: node.getGlyphPos(3))

    def testParaWords(self):
        root = self.loadEmptyScene()
        avg.LineNode(pos1=(0.5, 0), pos2=(0.5, 50), color="FF0000", parent=root)
        avg.LineNode(pos1=(119.5, 0.5), pos2=(119.5, 50), color="FF0000", parent=root)
        avg.LineNode(pos1=(74.5,60), pos2=(74.5, 110), color="FF0000", parent=root)
        avg.WordsNode(id="para", pos=(1,1), fontsize=12, width=70,
                font="Bitstream Vera Sans", text="Left-justified paragraph.", 
                parent=root)
        avg.WordsNode(id="paracenter", pos=(120,1), fontsize=12, width=70, 
                font="Bitstream Vera Sans", text="Centered paragraph", 
                alignment="center", parent=root)
        avg.WordsNode(id="pararight", pos=(75,60), fontsize=12, width=70,
                font="Bitstream Vera Sans", alignment="right",
                text="Right-justified paragraph.<i>l</i>",
                parent=root)
        avg.WordsNode(id="paralinespacing", pos=(80,60), fontsize=12, width=70,
                font="Bitstream Vera Sans", linespacing=-4,
                text="Paragraph with custom line spacing.",
                parent=root)
        self.start(True, [lambda: self.compareImage("testParaWords")])

    def testJustify(self):
        root = self.loadEmptyScene()
        avg.WordsNode(pos=(1,1), fontsize=12, font="Bitstream Vera Sans",
                variant="roman", justify=True, width=100,
                text="Justified paragraph more than one line long.", parent=root)
        self.start(True, [lambda: self.compareImage("testJustify")])

    def testWrapMode(self):
        def setCharMode():
            node.wrapmode = 'char'
        
        def setWordMode():
            node.wrapmode = 'word'
        
        def setWordCharMode():
            node.wrapmode = 'wordchar'
            
        root = self.loadEmptyScene()
        node = avg.WordsNode(pos=(1,1), fontsize=12, font="Bitstream Vera Sans",
                variant="roman", width=100, 
                text="""Wrapped paragraph more than one line long.
                        Withaverylongpackedlinewithnobreaks""",
                parent=root)
        self.start(True, 
                (lambda: self.compareImage("testWrapMode1"),
                 setCharMode,
                 lambda: self.compareImage("testWrapMode2"),
                 setWordMode,
                 lambda: self.compareImage("testWrapMode3"),
                 setWordCharMode,
                 lambda: self.compareImage("testWrapMode4"),
                ))

    def testWordsMask(self):
        def setMask():
            try:
                node.maskhref = "mask1.png"
            except RuntimeError:
                self.skip("no shader support")
                player.stop()
           
        def setColor():
            node.color = "FFFF00"

        def setOpacity():
            node.opacity = 0.5

        def setSize():
            rect = avg.RectNode(pos=(39.5, 30.5), size=(80, 60))
            root.appendChild(rect)
            node.masksize = (160, 120)
            node.opacity = 1 

        def setPos():
            node.pos = (40, 20)
            node.maskpos = (-40, -20)

        def setDefaultSize():
            node.masksize = (0,0)

        def setCentered():
            node.alignment = "center"
            node.masksize = (160, 120)
            node.pos = (80,20)
            node.maskpos = (0, -20)

        root = self.loadEmptyScene()
        node = avg.WordsNode(fontsize=8, linespacing=-4, font="Bitstream Vera Sans",
                variant="roman", width=160,
                text="Ich bin nur ein kleiner Blindtext. Wenn ich gross bin, will ich \
                    Ulysses von James Joyce werden. Aber jetzt lohnt es sich noch nicht, \
                    mich weiterzulesen. Denn vorerst bin ich nur ein kleiner Blindtext. \
                    Ich bin nur ein kleiner Blindtext. Wenn ich gross bin, will ich \
                    Ulysses von James Joyce werden. Aber jetzt lohnt es sich noch nicht, \
                    mich weiterzulesen. Denn vorerst bin ich nur ein kleiner Blindtext. \
                    Ich bin nur ein kleiner Blindtext. Wenn ich gross bin, will ich \
                    Ulysses von James Joyce werden. Aber jetzt lohnt es sich noch nicht, \
                    mich weiterzulesen. Denn vorerst bin ich nur ein kleiner Blindtext.",
                parent=root)
        self.start(True,
                (setMask,
                 lambda: self.compareImage("testWordsMask1"),
                 setColor,
                 lambda: self.compareImage("testWordsMask2"),
                 setOpacity,
                 lambda: self.compareImage("testWordsMask3"),
                 setSize,
                 lambda: self.compareImage("testWordsMask4"),
                 setPos,
                 lambda: self.compareImage("testWordsMask5"),
                 setDefaultSize,
                 lambda: self.compareImage("testWordsMask6"),
                 setCentered,
                 lambda: self.compareImage("testWordsMask7"),
                ))

    def testHinting(self):
        def checkPositions():
#            node0 = root.getChild(0)
#            for i in range(len(node0.text)):
#                print node0.getGlyphPos(i)
            noHint = root.getChild(0)
            hint = root.getChild(1)
            posNoHint = noHint.getGlyphPos(6)
            posHint = hint.getGlyphPos(6)
            self.assertNotEqual(posNoHint, posHint)
            noHint.hint = True
            hint.hint = False
            self.assertEqual(posNoHint, hint.getGlyphPos(6))
            self.assertEqual(posHint, noHint.getGlyphPos(6))

        if platform.system() == "Linux":
            self.skip("Linux support requires modified font config")
        else:
            root = self.loadEmptyScene()
            avg.WordsNode(pos=(1,1), fontsize=12, font="Bitstream Vera Sans",
                    variant="roman", hint=False, text="Lorem ipsum dolor (no hinting)",
                    parent=root)
            avg.WordsNode(pos=(1,15), fontsize=12, font="Bitstream Vera Sans",
                    variant="roman", hint=True, text="Lorem ipsum dolor (hinting)",
                    parent=root)
            self.start(True, [checkPositions])


    def testSpanWords(self):
        def setTextAttrib():
            self.baselineBmp = player.screenshot()
            player.getElementByID("words").text = self.text
        
        def checkSameImage():
            bmp = player.screenshot()
            self.assert_(self.areSimilarBmps(bmp, self.baselineBmp, 0, 0))
        
        def createUsingDict():
            player.getElementByID("words").unlink()
            node = avg.WordsNode(id="words", pos=(1,1), fontsize=12, width=120,
                    font="Bitstream Vera Sans", variant="roman", text=self.text)
            root.appendChild(node)
        
        self.text = """
              Markup: 
              <span size='14000' rise='5000' foreground='red'>span</span>, 
              <i>italics</i>, <b>bold</b>
        """
        root = self.loadEmptyScene()
        node = player.createNode("""
            <words id="words" x="1" y="1" fontsize="12" width="120" 
                font="Bitstream Vera Sans" variant="roman">
        """
        +self.text+
        """
            </words>
        """)
        root.appendChild(node)
        self.start(True, 
                [lambda: self.compareImage("testSpanWords"),
                 setTextAttrib,
                 lambda: self.compareImage("testSpanWords"),
                 checkSameImage,
                 createUsingDict,
                 lambda: self.compareImage("testSpanWords"),
                 checkSameImage,
                ])
    
    def testDynamicWords(self):
        def changeText():
            oldwidth = words.width
            words.text = "blue" 
            self.assertNotEqual(words.width, oldwidth)
            words.x += 10

        def changeColor():
            words.color = "404080"

        def activateText():
            words.active = True
        
        def deactivateText():
            words.active = False
        
        def changeFont():
            words.font = "Bitstream Vera Sans"
            words.height = 0
            words.fontsize = 30
        
        def changeFont2():
            words.fontsize = 18
        
        def changeTextWithInvalidTag():
            try:
                words.text = "This <invalid_tag/>bombs"
            except:
                words.text = "except"
            self.assertEqual(words.text, "except")
       
        root = self.loadEmptyScene()
        words = avg.WordsNode(pos=(1,1), fontsize=12, font="Bitstream Vera Sans",
                text="foo", parent=root)
        self.start(True, 
                (lambda: self.compareImage("testDynamicWords1"),
                 changeText,
                 lambda: self.compareImage("testDynamicWords2"),
                 changeColor,
                 lambda: self.compareImage("testDynamicWords3"),
                 changeFont,
                 lambda: self.compareImage("testDynamicWords4"),
                 deactivateText,
                 lambda: self.compareImage("testDynamicWords5"),
                 activateText,
                 changeFont2,
                 lambda: self.compareImage("testDynamicWords6"),
                 changeTextWithInvalidTag
                ))

    def testI18NWords(self):
        def changeUnicodeText():
            words.text = "Arabic nonsense: ﯿﭗ"
        
        def setNBSP():
            words.width=100
            words.text=(u"blindtext1\u00A0blindtext2\u00Ablindtext3 "+
                    u"blindtext4\u00A0blindtext\u00A0blindtext\u00A0")

        root = self.loadEmptyScene()
        avg.WordsNode(pos=(1,1), fontsize=14, font="Bitstream Vera Sans",
                text="一二三四五六七八九", parent=root)
        words = avg.WordsNode(pos=(1,24), fontsize=12, font="Bitstream Vera Sans",
                text="foo", parent=root)
        root.appendChild(
                player.createNode("""
                    <words x="1" y="48" fontsize="12" font="Bitstream Vera Sans">
                            &amp;
                    </words>
                """))
        avg.WordsNode(pos=(12,48), fontsize=12, font="Bitstream Vera Sans", text="&amp;",
                rawtextmode=True, parent=root)

        self.start(True, 
                (lambda: self.compareImage("testI18NWords1"),
                 changeUnicodeText,
                 lambda: self.compareImage("testI18NWords2"),
                 setNBSP,
                 lambda: self.compareImage("testI18NWords3"),
                ))

    def testRawText(self):
        def createDynNodes():
            self.dictdnode = avg.WordsNode(text='&lt;test dyndict&amp;', 
                    rawtextmode=True, pos=(1,65), font='Bitstream Vera Sans', 
                    variant='roman', fontsize=12)
            root.appendChild(self.dictdnode)

            self.xmldnode = player.createNode("""
                <words text="&lt;test dynattr&amp;" fontsize="12" 
                        font="Bitstream Vera Sans" variant="roman" rawtextmode="true"
                        x="1" y="85"/>""")
            root.appendChild(self.xmldnode)
        
        def switchRawMode():
            self.dictdnode.rawtextmode = False
            valNode.rawtextmode = True
            attribNode.rawtextmode = True
        
        def bombIt():
            def cantRun():
                self.xmldnode.rawtextmode = False
            self.assertRaises(RuntimeError, cantRun)
        
        def assignNewTexts():
            text = u'&ùùààxx>'
            self.dictdnode.rawtextmode = True
            self.dictdnode.text = text
            self.xmldnode.text = text
            valNode.text = text
            attribNode.text = text

        root = self.loadEmptyScene()
        attribNode = avg.WordsNode(text="ùnicòdé <b>bold</b>",
                fontsize=12, pos=(1,5), font="Bitstream Vera Sans", parent=root)
        valNode = player.createNode("""
            <words id="nodeval" fontsize="10" x="1" y="25" font="Bitstream Vera Sans"><b>bold</b> ùnicòdé  &lt;</words>""")
        root.appendChild(valNode)
        root.appendChild(
                player.createNode("""
                        <words x="1" y="45" fontsize="15" font="Bitstream Vera Sans">
                            &amp;
                        </words>"""))

        self.start(True, 
                (lambda: self.compareImage("testRawText1"),
                 createDynNodes,
                 lambda: self.compareImage("testRawText2"),
                 switchRawMode,
                 lambda: self.compareImage("testRawText3"),
                 bombIt,
                 assignNewTexts,
                 lambda: self.compareImage("testRawText4"),
                ))

    def testWordsBR(self):
        root = self.loadEmptyScene()
        avg.WordsNode(pos=(1,1), fontsize=12, font="Bitstream Vera Sans", variant="roman",
               text="paragraph 1<br/>paragraph 2", parent=root)
        self.start(True, 
                [lambda: self.compareImage("testWordsBR")])

    def testLetterSpacing(self):
        def setSpacing():
            player.getElementByID("words1").letterspacing=-2
            player.getElementByID("words2").letterspacing=-2
        
        root = self.loadEmptyScene()
        avg.WordsNode(id="words1", pos=(1,1), fontsize=12, font="Bitstream Vera Sans",
                variant="roman",
                text="""normal
                   <span letter_spacing="-2048"> packed</span>
                   <span letter_spacing="2048"> spaced</span>""",
                parent=root)
        avg.WordsNode(id="words2", pos=(1,20), fontsize=12, font="Bitstream Vera Sans",
                variant="roman", letterspacing=2, text="spaced", parent=root)
        self.start(True, 
                (lambda: self.compareImage("testLetterSpacing1"),
                 setSpacing,
                 lambda: self.compareImage("testLetterSpacing2")
                ))
       
    def testPositioning(self):
        def click(pos):
            self.fakeClick(int(pos[0]), int(pos[1]))
           
        def testInside(bInside):
            ok = bInside == self.clicked
            self.clicked = False
            return ok

        def onMouse(event):
            self.clicked = True

        root = self.loadEmptyScene()
        avg.LineNode(pos1=(4, 20.5), pos2=(157, 20.5), color="FF0000", parent=root)
        avg.LineNode(pos1=(4.5, 20.5), pos2=(4.5, 110), color="FF0000", parent=root)
        avg.LineNode(pos1=(156.5, 20.5), pos2=(156.5, 110), color="FF0000", parent=root)
        avg.LineNode(pos1=(80.5, 20.5), pos2=(80.5, 110), color="FF0000", parent=root)
        avg.WordsNode(id="left", pos=(4,20), fontsize=12, font="Bitstream Vera Sans",
                variant="roman", text="Norm", parent=root)
        avg.WordsNode(pos=(45,20), fontsize=12, font="Bitstream Vera Sans",
                variant="roman", text="orm", parent=root)
        avg.WordsNode(pos=(75,20), fontsize=12, font="Bitstream Vera Sans",
                variant="roman", text="ÖÄÜ", parent=root)
        avg.WordsNode(pos=(4,40), fontsize=12, font="Bitstream Vera Sans",
                variant="oblique", text="Jtalic", parent=root)
        avg.WordsNode(id="right", pos=(156,60), fontsize=12, alignment="right",
                font="Bitstream Vera Sans", variant="roman", text="Right-aligned",
                parent=root)
        avg.WordsNode(id="center", pos=(80,80), fontsize=12, alignment="center",
                font="Bitstream Vera Sans", variant="roman", text="Centered",
                parent=root)
        for id in ["left", "center", "right"]:
            player.getElementByID(id).subscribe(avg.Node.CURSOR_DOWN, onMouse)
        self.clicked = False
        leftWidth = player.getElementByID("left").getMediaSize()[0]
        centerWidth = player.getElementByID("center").getMediaSize()[0]
        rightWidth = player.getElementByID("right").getMediaSize()[0]

        self.start(True, 
                (lambda: self.compareImage("testPositioning"),
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
        def testColor(col):
            avg.WordsNode(color=col)
        
        def assignValidColor():
            testColor('123456')
        
        def assignInvalidColor1():
            testColor('1234567')
        
        def assignInvalidColor2():
            testColor('xxx')
        
        def assignInvalidColor3():
            testColor('xxxxxx')

        self.loadEmptyScene()
        self.start(True, 
                (self.assertRaises(RuntimeError, assignInvalidColor1),
                 self.assertRaises(RuntimeError, assignInvalidColor2),
                 self.assertRaises(RuntimeError, assignInvalidColor3),
                ))

    def testFontDir(self):
        avg.WordsNode.addFontDir('extrafonts')
        root = self.loadEmptyScene()
        avg.WordsNode(font="testaddfontdir", fontsize=50, text="ABAAA", parent=root)
        self.start(True, 
                (lambda: self.compareImage("testFontDir"),))
    
    def testGetNumLines(self):
        textNode = avg.WordsNode(text="paragraph 1<br/>paragraph 2<br/>paragraph 3")
        self.assertEqual(textNode.getNumLines(), 3)
        textNode.text = ""
        self.assertEqual(textNode.getNumLines(), 0)
    
    def testGetLineExtents(self):        
        textNode = avg.WordsNode(fontsize = 100,
                                font = "Bitstream Vera Sans",
                                text = "bla <br/> blabli <br/> blabliblabla")
        self.assertEqual(textNode.getLineExtents(0), (184,117))
        self.assertEqual(textNode.getLineExtents(1), (303,117))
        
    def testGetCharIndexFromPos(self):
        textNode = avg.WordsNode(fontsize=30,
                      font = "Bitstream Vera Sans",
                      text = "A B C D E F G H Ä Ö Ü ? Ì Á Í Å Ø ∏ ~ ç Ç Å",
                      width = 300)

        for k in (1,2,3,23,42):
            pos = textNode.getGlyphPos(k)
            char = textNode.getCharIndexFromPos(pos)
            self.assertEqual(char, k)
        
    def testGetTextAsDisplayed(self):
        orgText = "A<br/>B C <b>D</b> E F G H <i>Ä</i> Ö Ü ? Ì Á<br/>Í Å Ø ∏ ~ ç Ç Å"
        orgTextWithout = "A\nB C D E F G H Ä Ö Ü ? Ì Á\nÍ Å Ø ∏ ~ ç Ç Å"
        textNode = avg.WordsNode(fontsize=30,
                      font = "Bitstream Vera Sans",
                      text = orgText,
                      width = 300)
        self.assertEqual(orgTextWithout, textNode.getTextAsDisplayed())

    def testSetWidth(self):
        root = self.loadEmptyScene()
        text = "42 " * 42
        textNode = avg.WordsNode(
                parent=root,
                fontsize = 10,
                font = "Bitstream Vera Sans",
                text = text)
        
        def testSize(p1, p2):
            self.assert_(abs(p1.x - p2.x) < 5)
            self.assert_(abs(p1.y - p2.y) < 50)
        
        testSize(textNode.size, avg.Point2D(630,13))
        testSize(textNode.getMediaSize(), avg.Point2D(630,13))
        mediaSize = textNode.getMediaSize()

        def changeSize():
            textNode.width = 50
            testSize(textNode.size, avg.Point2D(50,182))
            testSize(textNode.getMediaSize(), avg.Point2D(45,182))
            self.assertNotEqual(mediaSize, textNode.getMediaSize())

        self.start(True, 
                [lambda: changeSize()])
        
    def testTooWide(self):
        root = self.loadEmptyScene()
        text = "42 " * 42 * 20 
        avg.WordsNode(parent=root, text=text)
        self.assertRaises(RuntimeError, lambda: self.start(True, (None, None)))

    def testWordsGamma(self):
        
        def setGamma():
            node.aagamma = 4

        root = self.loadEmptyScene()
        for i, gamma in enumerate((2, 1.5, 1)):
            node = avg.WordsNode(pos=(1,i*20), fontsize=12, font="Bitstream Vera Sans", 
                    variant="roman", aagamma=gamma, text="lorem ipsum dolor", 
                    parent=root)
        self.start(True, 
                (lambda: self.compareImage("testWordsGamma1"),
                 setGamma,
                 lambda: self.compareImage("testWordsGamma2"),
                ))


def wordsTestSuite(tests):
    availableTests = (
            "testSimpleWords",
            "testRedrawOnDemand",
            "testFontStyle",
            "testBaseStyle",
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
            "testGetNumLines",
            "testGetLineExtents",
            "testGetCharIndexFromPos",
            "testGetTextAsDisplayed",
            "testSetWidth",
            "testTooWide",
            "testWordsGamma",
            )
    return createAVGTestSuite(availableTests, WordsTestCase, tests)
