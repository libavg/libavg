
MOZILLA_DIR = `mozilla-config --prefix`
MOZILLA_LIB_DIR    =  $(MOZILLA_DIR)/lib
MOZILLA_BIN_DIR    = $(MOZILLA_DIR)/lib/mozilla
MOZILLA_COMPONENT_DIR = $(MOZILLA_BIN_DIR)/components
MOZILLA_IDL_DIR = $(MOZILLA_DIR)/share/idl/mozilla

INCLUDES = `mozilla-config --cflags xpcom nspr` \
           -I/usr/include/g++ \
           `xml2-config --cflags` \
           -I_xpidlgen \
           `directfb-config --cflags` \
           `paintlib-config --cflags` 

ALL_H = AVGAVGNode.h AVGContainer.h  AVGDFBDisplayEngine.h AVGEvent.h \
          AVGException.h AVGExcl.h AVGFramerateManager.h AVGImage.h \
          AVGNode.h AVGPlayer.h AVGTimeout.h AVGRegion.h \
          AVGVisibleNode.h IAVGEvent.h IAVGNode.h IAVGPlayer.h \
          IDisplayEngine.h IJSEvalKruecke.h XMLHelper.h AVGTime.h \
          AVGWords.h

libavg_LTLIBRARIES = libavg.la

libavg_la_LDFLAGS = `mozilla-config --libs nspr` `paintlib-config --libs` \
                    `xml2-config --libs` `directfb-config --libs`

libavg_la_SOURCES = AVGAVGNode.cpp AVGExcl.cpp \
                    AVGContainer.cpp AVGFramerateManager.cpp AVGTimeout.cpp \
                    AVGDFBDisplayEngine.cpp AVGImage.cpp AVGVisibleNode.cpp \
                    AVGEvent.cpp AVGNode.cpp XMLHelper.cpp AVGRegion.cpp\
                    AVGException.cpp AVGPlayer.cpp XPAVGPlayerModule.cpp \
                    AVGTime.cpp AVGWords.cpp \
                    $(ALL_H) 
BUILT_SOURCES = IAVGPlayer.h IJSEvalKruecke.h IAVGEvent.h IAVGNode.h

%.h : %.idl
	$(MOZILLA_BIN_DIR)/xpidl -m header -I $(MOZILLA_IDL_DIR) -w -o $* $<
	$(MOZILLA_BIN_DIR)/xpidl -m typelib -I $(MOZILLA_IDL_DIR) -w -o $* $<

libavgdir = $(includedir)/avg
libavgHEADERS = $(ALL_H)


