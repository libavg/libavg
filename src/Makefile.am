
#MOZILLA_DIR = $(PRO)/extern/mozilla
MOZILLA_DIR = `mozilla-config --prefix`
#MOZILLA_INCLUDE_DIR = $(MOZILLA_DIR)/include
#MOZILLA_INCLUDES   = -I $(MOZILLA_INCLUDE_DIR)/ \
#                     -I $(MOZILLA_INCLUDE_DIR)/xpcom \
#                     -I $(MOZILLA_INCLUDE_DIR)/nspr
MOZILLA_LIB_DIR    =  $(MOZILLA_DIR)/lib
#MOZILLA_LDFLAGS    = -L$(MOZILLA_LIB_DIR) \
#                     -lnspr4 -lplds4
# -lembedstring -lxpcomglue

#MOZILLA_BIN_DIR    = $(MOZILLA_DIR)/bin
MOZILLA_BIN_DIR    = $(MOZILLA_DIR)/lib/mozilla
MOZILLA_COMPONENT_DIR = $(MOZILLA_BIN_DIR)/components
#MOZILLA_IDL_DIR = $(MOZILLA_DIR)/idl/
MOZILLA_IDL_DIR = $(MOZILLA_DIR)/share/idl/mozilla

#INCLUDES = $(MOZILLA_INCLUDES) 
INCLUDES = `mozilla-config --cflags xpcom nspr` \
           -I/usr/include/g++ \
           `xml2-config --cflags` \
           -I_xpidlgen \
           `directfb-config --cflags` \
	   `paintlib-config --cflags`

ALL_H = AVGAVGNode.h AVGContainer.h  AVGDFBDisplayEngine.h AVGEvent.h \
          AVGException.h AVGExcl.h AVGFramerateManager.h AVGImage.h \
          AVGNode.h AVGPlayer.h AVGSDLDisplayEngine.h AVGTimeout.h \
          AVGVisibleNode.h IAVGEvent.h IAVGNode.h IAVGPlayer.h \
          IDisplayEngine.h IJSEvalKruecke.h XMLHelper.h

libavg_LTLIBRARIES = libavg.la

libavg_la_LDFLAGS = `mozilla-config --libs nspr` `paintlib-config --libs` \
                    `xml2-config --libs` `directfb-config --libs`

libavg_la_SOURCES = AVGAVGNode.cpp AVGExcl.cpp AVGSDLDisplayEngine.cpp \
                    AVGContainer.cpp AVGFramerateManager.cpp AVGTimeout.cpp \
                    AVGDFBDisplayEngine.cpp AVGImage.cpp AVGVisibleNode.cpp \
                    AVGEvent.cpp AVGNode.cpp XMLHelper.cpp \
                    AVGException.cpp AVGPlayer.cpp XPAVGPlayerModule.cpp \
                    $(ALL_H) 
BUILT_SOURCES = IAVGPlayer.h IJSEvalKruecke.h IAVGEvent.h IAVGNode.h

%.h : %.idl
	$(MOZILLA_BIN_DIR)/xpidl -m header -I $(MOZILLA_IDL_DIR) -w -o $* $<
	$(MOZILLA_BIN_DIR)/xpidl -m typelib -I $(MOZILLA_IDL_DIR) -w -o $* $<

libavgdir = $(includedir)/avg
libavgHEADERS = $(ALL_H)


