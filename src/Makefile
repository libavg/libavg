#
# $Id$
#

CPPFLAGS=    -shared    
#	 -fno-rtti       
#         -fno-exceptions 
#            -shared

GECKO_CONFIG_INCLUDE = -include mozilla-config.h

GECKO_DEFINES = #-DXPCOM_GLUE #-DMOZILLA_STRICT_API

GECKO_INCLUDE_DIR=$(PRO)/extern/mozilla/include
GECKO_INCLUDES = -I $(GECKO_INCLUDE_DIR)/         \
                 -I $(GECKO_INCLUDE_DIR)/xpcom    \
                 -I $(GECKO_INCLUDE_DIR)/nspr     
#                 -I $(GECKO_INCLUDE_DIR)/embedstring
#                 -I $(GECKO_INCLUDE_DIR)/string   \

LOCAL_INCLUDES=-I/usr/include/g++ -I/usr/include/libxml2/ -I/usr/include/SDL \
	           -I_xpidlgen `directfb-config --cflags`
				 
GECKO_LIB_DIR = $(PRO)/extern/mozilla/lib
GECKO_LDFLAGS = -L $(GECKO_LIB_DIR) \
                -lnspr4         \
                -lplds4         \
#                -lembedstring   
#				-lxpcomglue     

GECKO_BIN_DIR = $(PRO)/extern/mozilla/bin

LOCAL_LDFLAGS = -lSDL -lpthread -lxml2 -lpaintlib -ltiff -lungif -lz -lpng \
		-ljpeg -lcurl `directfb-config --libs`
				
CPPSRCS   =  AVGPlayer.cpp AVGNode.cpp \
        AVGAVGNode.cpp AVGVisibleNode.cpp \
        AVGContainer.cpp AVGException.cpp \
        XMLHelper.cpp AVGImage.cpp \
        AVGDFBDisplayEngine.cpp XPAVGPlayerModule.cpp \
		AVGFramerateManager.cpp AVGTimeout.cpp \
		AVGEvent.cpp AVGExcl.cpp \
		$(NULL)

XPIDLSRCS = IAVGPlayer.idl IJSEvalKruecke.idl IAVGEvent.idl IAVGNode.idl
IDL_DIR = $(PRO)/extern/mozilla/idl/

GECKO_COMPONENT_DIR = $(GECKO_BIN_DIR)/components

%.h : %.idl
	$(GECKO_BIN_DIR)/xpidl -m header -I $(IDL_DIR) -w -o $* $<

%.xpt : %.idl	
	$(GECKO_BIN_DIR)/xpidl -m typelib -I $(IDL_DIR) -w -o $* $<
	

build: IAVGPlayer.h IJSEvalKruecke.h IAVGEvent.h IAVGNode.h \
	   IAVGPlayer.xpt IJSEvalKruecke.xpt IAVGEvent.xpt IAVGNode.xpt 
	c++ -o libavgplayer.so $(GECKO_CONFIG_INCLUDE) $(GECKO_DEFINES) \
		$(LOCAL_INCLUDES) $(LOCAL_LDFLAGS)                          \
		$(GECKO_INCLUDES) $(GECKO_LDFLAGS) $(CPPFLAGS) $(CPPSRCS)
	chmod a+x libavgplayer.so
	
install: 
	cp I*.xpt $(GECKO_COMPONENT_DIR)
	cp libavgplayer.so $(GECKO_COMPONENT_DIR)
	rm -f $(GECKO_COMPONENT_DIR)/xpti.dat
	rm -f $(GECKO_COMPONENT_DIR)/compreg.dat
	

#include $(topsrcdir)/config/config.mk
#LIBS    +=        \
#    $(XPCOM_LIBS) \
#    $(NSPR_LIBS)  \
#    $(NULL)
#
#LOCAL_INCLUDES=-I/usr/include/g++ -I/usr/include/libxml2/ -I/usr/include/SDL 
#CXXFLAGS += -fexceptions -frtti
#EXTRA_DSO_LDOPTS += $(MOZ_COMPONENT_LIBS) \
#    -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lSDL_gfx -lpthread \
#    -lxml2 -lpaintlib -ltiff -lungif -lz -lpng -ljpeg -lcurl \
#	-L$(DIST)/lib -lxpcomglue
#
#include $(topsrcdir)/config/rules.mk
#
#install:: $(TARGETS)
#	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/_xpidlgen/IAVGPlayer.xpt $(mozappdir)/components
#	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/_xpidlgen/IAVGEvent.xpt $(mozappdir)/components
#	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/_xpidlgen/IJSEvalKruecke.xpt $(mozappdir)/components
#	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/_xpidlgen/IAVGNode.xpt $(mozappdir)/components

