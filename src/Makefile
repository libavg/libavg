#
# $Id$
#
MOZILLA_ROOT=/usr/local/src/mozilla
DEPTH         = $(MOZILLA_ROOT)
topsrcdir     = $(MOZILLA_ROOT)
srcdir        = .
VPATH         = .

include $(DEPTH)/config/autoconf.mk

MODULE        = avgplayer
XPIDL_MODULE  = avgplayer
LIBRARY_NAME  = avgplayer
EXPORT_LIBRARY = 1
IS_COMPONENT  = 1
MODULE_NAME   = AVGPlayerModule
DEFINES       += -DXPCOM_GLUE
REQUIRES      = string \
                xpcom \
                avgplayer \
				$(NULL)
CPPSRCS   =  AVGPlayer.cpp AVGNode.cpp \
        AVGAVGNode.cpp AVGVisibleNode.cpp \
        AVGContainer.cpp AVGException.cpp \
        XMLHelper.cpp AVGImage.cpp \
        AVGSDLDisplayEngine.cpp XPAVGPlayerModule.cpp \
		AVGFramerateManager.cpp AVGTimeout.cpp \
		$(NULL)
XPIDLSRCS = IAVGPlayer.idl IJSEvalKruecke.idl
include $(topsrcdir)/config/config.mk
LIBS    +=        \
    $(XPCOM_LIBS) \
    $(NSPR_LIBS)  \
    $(NULL)

LOCAL_INCLUDES=-I/usr/include/g++ -I/usr/include/libxml2/ -I/usr/include/SDL 
CXXFLAGS += -fexceptions -frtti
EXTRA_DSO_LDOPTS += $(MOZ_COMPONENT_LIBS) \
    -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lSDL_gfx -lpthread \
    -lxml2 -lpaintlib -ltiff -lungif -lz -lpng -ljpeg -lcurl \
	-L$(DIST)/lib -lxpcomglue

include $(topsrcdir)/config/rules.mk

install:: $(TARGETS)
	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/_xpidlgen/IAVGPlayer.xpt $(mozappdir)/components
	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/_xpidlgen/IJSEvalKruecke.xpt $(mozappdir)/components
	$(SYSINSTALL) $(IFLAGS1) $(srcdir)/JSEvalKruecke.js $(mozappdir)/components
