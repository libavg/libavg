#!/usr/bin/python
# -*- coding: utf-8 -*-
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
            <words x="1" y="1" size="12" font="Bitstream Vera Sans" 
                text="Bitstream Vera Sans" variant="roman"/>
            <words id="sanstext" x="1" y="16" size="12" font="Bitstream Vera Sans" 
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
        node = Player.createNode("words", {"text":"Bold"})
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
        self.start("paratext.avg",
                [lambda: self.compareImage("testParaWords", True)])
   
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
                    "id":"words", "x":1, "y":1, "size":12, "parawidth":120,
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
            <words id="words" x="1" y="1" size="12" parawidth="120" 
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
            node.font = "Times New Roman"
            node.height = 0
            node.size = 30
        def changeFont2():
            node = Player.getElementByID("dynamictext")
            node.size = 18
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
            self.dictdnode = Player.createNode("words", {'text':'&lt;test dyndict&amp;', 'rawtextmode':True, 'x':1, 'y':65, 'font':'Bitstream Vera Sans', 'variant': 'roman', 'size':12})
            Player.getRootNode().appendChild(self.dictdnode)

            self.xmldnode = Player.createNode("<words text=\"&lt;test dynattr&amp;\" size=\"12\" font=\"Bitstream Vera Sans\" variant=\"roman\" rawtextmode=\"true\" x=\"1\" y=\"85\"/>")
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

def wordsTestSuite():
    suite = unittest.TestSuite()
    suite.addTest(WordsTestCase("testSimpleWords"))
    suite.addTest(WordsTestCase("testGlyphPos"))
    suite.addTest(WordsTestCase("testParaWords"))
    suite.addTest(WordsTestCase("testSpanWords"))
    suite.addTest(WordsTestCase("testDynamicWords"))
    suite.addTest(WordsTestCase("testI18NWords"))
    suite.addTest(WordsTestCase("testRawText"))
    return suite

Player = avg.Player.get()
