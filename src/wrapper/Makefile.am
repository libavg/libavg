
INCLUDES = -I. -I../player \
		@XML2_CFLAGS@ @PTHREAD_CFLAGS@ @PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @DC1394_2_CFLAGS@

ALL_H = WrapHelper.h raw_constructor.hpp

if APPLE
    XGL_LIBS =
    APPLE_LDFLAGS=-read_only_relocs suppress -F/System/Library/PrivateFrameworks -framework MultitouchSupport 
else
    XGL_LIBS = -lXxf86vm
    APPLE_LDFLAGS=
endif

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

pkgpyexec_LTLIBRARIES = avg.la
avg_la_SOURCES = WrapHelper.cpp avg_wrap.cpp node_wrap.cpp raster_wrap.cpp \
        event_wrap.cpp fx_wrap.cpp \
        device_wrap.cpp bitmap_wrap.cpp anim_wrap.cpp $(ALL_H) 
avg_la_LDFLAGS = $(APPLE_LDFLAGS) -module -XCClinker $(BOOST_PYTHON_LIBS) \
        -l@BOOST_THREAD_LIB@ @XML2_LIBS@ \
        @DC1394_2_LIBS@ @GLU_LIBS@ $(ALL_GL_LIBS) \
        @LIBFFMPEG@ $(MAGICK_LIBS) @PANGOFT2_LIBS@ @PTHREAD_LIBS@
avg_la_LIBADD = ../conradrelais/libconradrelais.la \
                ../player/libplayer.la \
                ../audio/libaudio.la \
                ../video/libvideo.la \
                ../graphics/libgraphics.la \
                ../base/libbase.la \
                ../imaging/libimaging.la \
                ../parport/libparport.la \
                ../anim/libanim.la \
                ../lmfit/liblmfit.la \
                ../oscpack/liboscpack.la
pkgpyexec_PYTHON = __init__.py
