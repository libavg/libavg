
INCLUDES = -I. \
           `mozilla-config --cflags xpcom nspr js xpconnect string` \
           -I/usr/include/g++ \
           `xml2-config --cflags` \
           -I_xpidlgen \
           `directfb-config --cflags` \
           `paintlib-config --cflags`

if ENABLE_1394
    DC1394_LIBS = -ldc1394_control -lraw1394
    DC1394_SOURCES = AVGCamera.cpp
    DC1394_INCLUDES = AVGCamera.h
    DC1394_IDL = IAVGCamera.h
else
    DC1394_LIBS =
    DC1394_SOURCES =
    DC1394_INCLUDES =
    DC1394_IDL =
endif

if ENABLE_GL
    GL_LIBS = -lGL -lGLU -lSDL_ttf 
    GL_SOURCES = AVGSDLDisplayEngine.cpp OGLHelper.cpp \
                 AVGSDLFontManager.cpp AVGSDLFont.cpp AVGOGLSurface.cpp
    GL_INCLUDES = AVGSDLDisplayEngine.h OGLHelper.h \
                  AVGSDLFontManager.h AVGOGLSurface.h \
                  AVGSDLFont.h
else
    GL_LIBS =
    GL_SOURCES =
    GL_INCLUDES =
endif

if ENABLE_DFB
    DFB_LIBS = `directfb-config --libs`
    DFB_SOURCES = AVGDFBDisplayEngine.cpp AVGDFBFontManager.cpp \
                  AVGDFBFont.cpp AVGDFBSurface.cpp 
    DFB_INCLUDES = AVGDFBDisplayEngine.h AVGDFBFontManager.h AVGDFBFont.h \
                  AVGDFBSurface.h
else
    DFB_LIBS =
    DFB_SOURCES =
    DFB_INCLUDES =
endif

ALL_H = AVGAVGNode.h AVGContainer.h AVGEvent.h \
          AVGException.h AVGExcl.h AVGFramerateManager.h AVGImage.h \
          AVGNode.h AVGPlayer.h AVGTimeout.h AVGRegion.h AVGRelais.h \
          IAVGEvent.h IAVGNode.h IAVGPlayer.h IAVGConradRelais.h \
          IDisplayEngine.h XMLHelper.h AVGTimeSource.h \
          AVGWords.h AVGFontManager.h AVGVideo.h xpshell.h \
          acJSContextPublisher.h AVGJSScript.h AVGLogger.h \
          IAVGFont.h FileHelper.h JSHelper.h AVGDivNode.h \
          IAVGVideoDecoder.h AVGFFMpegDecoder.h AVGVideoBase.h \
          $(DC1394_INCLUDES) $(GL_INCLUDES) $(DFB_INCLUDES)
          
libavg_PROGRAMS = xpshell
xpshell_SOURCES = xpshell.cpp FileHelper.cpp FileHelper.h xpshell.h acJSContextPublisher.h
xpshell_LDFLAGS = `mozilla-config --libs nspr xpcom` -lmozjs

libavg_LTLIBRARIES = libavg.la


libavg_la_LDFLAGS = `mozilla-config --libs nspr` `paintlib-config --libs` \
                    `xml2-config --libs` \
                    -lavformat -lavcodec \
                    $(DC1394_LIBS) $(GL_LIBS) $(DFB_LIBS)

libavg_la_SOURCES = AVGEvent.cpp AVGMouseEvent.cpp AVGKeyEvent.cpp \
                    AVGWindowEvent.cpp AVGEventDispatcher.cpp \
                    AVGDebugEventSink.cpp AVGAVGNode.cpp AVGExcl.cpp \
                    AVGContainer.cpp AVGFramerateManager.cpp AVGTimeout.cpp \
                    AVGImage.cpp \
                    AVGNode.cpp XMLHelper.cpp AVGRegion.cpp\
                    AVGException.cpp AVGPlayer.cpp XPAVGPlayerModule.cpp \
                    AVGTimeSource.cpp AVGWords.cpp AVGFontManager.cpp\
                    AVGVideo.cpp acJSContextPublisher.cpp AVGJSScript.cpp\
                    AVGLogger.cpp AVGConradRelais.cpp \
                    FileHelper.cpp JSHelper.cpp AVGDivNode.cpp\
                    AVGFFMpegDecoder.cpp AVGVideoBase.cpp \
                    $(DC1394_SOURCES) $(GL_SOURCES) $(DFB_SOURCES)\
                    $(ALL_H) 
 
BUILT_SOURCES = IAVGPlayer.h IAVGEvent.h IAVGNode.h IAVGVideo.h \
                acIJSContextPublisher.h IAVGWords.h IAVGExcl.h \
                IAVGConradRelais.h IAVGKeyEvent.h IAVGWindowEvent.h \
                IAVGMouseEvent.h IAVGVideoBase.h $(DC1394_IDL)

CLEANFILES = $(BUILT_SOURCES)

%.h : %.idl
	./avg_xpidl $* $< 

libavgdir = $(includedir)/avg
libavgHEADERS = $(ALL_H)

install:
	bash ../install_avg $(bindir) $(includedir)
