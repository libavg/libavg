
MOZILLA_DIR = `mozilla-config --prefix`
#MOZILLA_LIB_DIR    =  $(MOZILLA_DIR)/lib
MOZILLA_LIB_DIR    =  $(MOZILLA_DIR)/lib/mozilla-1.4
MOZILLA_BIN_DIR    = $(MOZILLA_DIR)/lib/mozilla
MOZILLA_COMPONENT_DIR = $(MOZILLA_BIN_DIR)/components
#MOZILLA_IDL_DIR = $(MOZILLA_DIR)/share/idl
MOZILLA_IDL_DIR = $(MOZILLA_DIR)/share/idl/mozilla-1.4

INCLUDES = `mozilla-config --cflags xpcom nspr js xpconnect string` \
           -I/usr/include/g++ \
           `xml2-config --cflags` \
           -I_xpidlgen \
           `directfb-config --cflags` \
           `paintlib-config --cflags` 

ALL_H = AVGAVGNode.h AVGContainer.h  AVGDFBDisplayEngine.h AVGEvent.h \
          AVGException.h AVGExcl.h AVGFramerateManager.h AVGImage.h \
          AVGNode.h AVGPlayer.h AVGTimeout.h AVGRegion.h \
          AVGVisibleNode.h IAVGEvent.h IAVGNode.h IAVGPlayer.h \
          IDisplayEngine.h XMLHelper.h AVGTime.h \
          AVGWords.h AVGFontManager.h AVGVideo.h xpshell.h \
          acJSContextPublisher.h AVGJSScript.h

libavg_PROGRAMS = xpshell
xpshell_SOURCES =  xpshell.cpp xpshell.h acJSContextPublisher.h
xpshell_LDFLAGS =  `mozilla-config --libs nspr xpcom` -lmozjs


libavg_LTLIBRARIES = libavg.la

libavg_la_LDFLAGS =  -lmpeg3 `mozilla-config --libs nspr` `paintlib-config --libs` \
                    `xml2-config --libs` `directfb-config --libs`

libavg_la_SOURCES = AVGAVGNode.cpp AVGExcl.cpp \
                    AVGContainer.cpp AVGFramerateManager.cpp AVGTimeout.cpp \
                    AVGDFBDisplayEngine.cpp AVGImage.cpp AVGVisibleNode.cpp \
                    AVGEvent.cpp AVGNode.cpp XMLHelper.cpp AVGRegion.cpp\
                    AVGException.cpp AVGPlayer.cpp XPAVGPlayerModule.cpp \
                    AVGTime.cpp AVGWords.cpp AVGFontManager.cpp\
                    AVGVideo.cpp acJSContextPublisher.cpp AVGJSScript.cpp\
                    $(ALL_H) 
BUILT_SOURCES = IAVGPlayer.h IAVGEvent.h IAVGNode.h IAVGVideo.h \
                acIJSContextPublisher.h

%.h : %.idl
	$(MOZILLA_LIB_DIR)/xpidl -m header -I $(MOZILLA_IDL_DIR) -w -o $* $<
	$(MOZILLA_LIB_DIR)/xpidl -m typelib -I $(MOZILLA_IDL_DIR) -w -o $* $<

libavgdir = $(includedir)/avg
libavgHEADERS = $(ALL_H)


