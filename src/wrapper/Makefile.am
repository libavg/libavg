AM_CPPFLAGS = -I. -I../player \
        @XML2_CFLAGS@ @PTHREAD_CFLAGS@ @PANGOFT2_CFLAGS@ @PYTHON_CPPFLAGS@ \
        @DC1394_2_CFLAGS@ @LIBRSVG_CFLAGS@ @FONTCONFIG_CFLAGS@

ALL_H = WrapHelper.h raw_constructor.hpp

if APPLE
    XGL_LIBS =
    APPLE_LDFLAGS = -read_only_relocs suppress -F/System/Library/PrivateFrameworks \
            -framework MultitouchSupport
else
if ENABLE_RPI
    XGL_LIBS = -lXxf86vm -lX11 -lGLESv2 -lEGL
else
if ENABLE_EGL
    XGL_LIBS = -lXxf86vm -lX11 -lGLESv2 -lEGL
else
    XGL_LIBS = -lXxf86vm -lX11
endif
endif
    APPLE_LDFLAGS =
endif

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

pkgpyexec_LTLIBRARIES = avg.la
avg_la_SOURCES = WrapHelper.cpp avg_wrap.cpp node_wrap.cpp raster_wrap.cpp \
        event_wrap.cpp fx_wrap.cpp \
        bitmap_wrap.cpp anim_wrap.cpp $(ALL_H)
avg_la_LDFLAGS = $(APPLE_LDFLAGS) -module -XCClinker
avg_la_LIBADD = ../player/libplayer.la \
                ../audio/libaudio.la \
                ../video/libvideo.la \
                ../graphics/libgraphics.la \
                ../base/libbase.la \
                ../base/triangulate/libtriangulate.la \
                ../imaging/libimaging.la \
                ../anim/libanim.la \
                ../lmfit/liblmfit.la \
                ../oscpack/liboscpack.la \
                @PANGOFT2_LIBS@ @LIBRSVG_LIBS@ $(BOOST_PYTHON_LIBS) \
                @BOOST_THREAD_LIBS@ @XML2_LIBS@ \
                @DC1394_2_LIBS@ @GLU_LIBS@ $(XI2_1_LIBS) $(XI2_2_LIBS) \
                $(ALL_GL_LIBS) @LIBFFMPEG@ @LIBAVRESAMPLE@ @PTHREAD_LIBS@ \
                @GDK_PIXBUF_LIBS@ @FONTCONFIG_LIBS@
