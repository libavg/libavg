
INCLUDES = `mozilla-config --cflags xpcom nspr js xpconnect string` \
           -I/usr/include/g++ \
           `xml2-config --cflags` \
           -I_xpidlgen \
           `directfb-config --cflags` \
           `paintlib-config --cflags` \
           -I/usr/include/libmpeg3 

ALL_H = AVGAVGNode.h AVGContainer.h  AVGDFBDisplayEngine.h AVGEvent.h \
          AVGException.h AVGExcl.h AVGFramerateManager.h AVGImage.h \
          AVGNode.h AVGPlayer.h AVGTimeout.h AVGRegion.h AVGRelais.h \
          IAVGEvent.h IAVGNode.h IAVGPlayer.h IAVGConradRelais.h \
          IDisplayEngine.h XMLHelper.h AVGTimeSource.h \
          AVGWords.h AVGFontManager.h AVGVideo.h xpshell.h \
          acJSContextPublisher.h AVGJSScript.h AVGLogger.h \
          AVGDFBFontManager.h IAVGFont.h AVGDFBFont.h FileHelper.h JSHelper.h \
          AVGSDLDisplayEngine.h AVGOGLBmp.h OGLHelper.h AVGSDLFontManager.h \
          AVGSDLFont.h IAVGVideoDecoder.h AVGFFMpegDecoder.h AVGVideoBase.h

libavg_PROGRAMS = xpshell
xpshell_SOURCES = xpshell.cpp FileHelper.cpp FileHelper.h xpshell.h acJSContextPublisher.h
xpshell_LDFLAGS = `mozilla-config --libs nspr xpcom` -lmozjs

libavg_LTLIBRARIES = libavg.la

libavg_la_LDFLAGS =  -lmpeg3 `mozilla-config --libs nspr` `paintlib-config --libs` \
                    `xml2-config --libs` `directfb-config --libs` \
                    -lGL -lGLU -lSDL_ttf -lavformat -lavcodec

libavg_la_SOURCES = AVGEvent.cpp AVGMouseEvent.cpp AVGKeyEvent.cpp AVGWindowEvent.cpp \
                    AVGEventDispatcher.cpp AVGDebugEventSink.cpp AVGAVGNode.cpp AVGExcl.cpp \
                    AVGContainer.cpp AVGFramerateManager.cpp AVGTimeout.cpp \
                    AVGDFBDisplayEngine.cpp AVGImage.cpp \
                    AVGNode.cpp XMLHelper.cpp AVGRegion.cpp\
                    AVGException.cpp AVGPlayer.cpp XPAVGPlayerModule.cpp \
                    AVGTimeSource.cpp AVGWords.cpp AVGFontManager.cpp\
                    AVGVideo.cpp acJSContextPublisher.cpp AVGJSScript.cpp\
                    AVGLogger.cpp AVGConradRelais.cpp AVGDFBFontManager.cpp\
                    AVGDFBFont.cpp FileHelper.cpp JSHelper.cpp AVGSDLDisplayEngine.cpp\
                    AVGOGLBmp.cpp OGLHelper.cpp AVGSDLFontManager.cpp \
                    AVGSDLFont.cpp AVGFFMpegDecoder.cpp AVGVideoBase.cpp \
                    $(ALL_H) 
 
BUILT_SOURCES = IAVGPlayer.h IAVGEvent.h IAVGNode.h IAVGVideo.h \
                acIJSContextPublisher.h IAVGWords.h IAVGExcl.h \
                IAVGConradRelais.h IAVGKeyEvent.h IAVGWindowEvent.h \
                IAVGMouseEvent.h IAVGVideoBase.h
CLEANFILES = $(BUILT_SOURCES)

%.h : %.idl
	./avg_xpidl $* $< 

libavgdir = $(includedir)/avg
libavgHEADERS = $(ALL_H)

install:
	bash ../install_avg $(bindir) $(includedir)
