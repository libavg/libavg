
INCLUDES = -I. -I../player \
		@XML2_CFLAGS@ @PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@

ALL_H = 

if ENABLE_1394
    DC1394_LIBS = -ldc1394_control -lraw1394
else
    DC1394_LIBS =
endif

if APPLE
    XGL_LIBS =
else
    XGL_LIBS = -lXxf86vm
endif

if ENABLE_GL
    ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)
else
    ALL_GL_LIBS =
endif

if ENABLE_DFB
    DFB_LIBS = @DIRECTFB_LIBS@
else
    DFB_LIBS =
endif

pkgpyexec_LTLIBRARIES = avg.la
avg_la_SOURCES = avg_wrap.cpp raster_wrap.cpp event_wrap.cpp device_wrap.cpp bitmap_wrap.cpp 
avg_la_LDFLAGS = -module -XCClinker -lboost_python @XML2_LIBS@ \
        $(DC1394_LIBS) @GLU_LIBS@ $(ALL_GL_LIBS) $(DFB_LIBS) \
		`pkg-config --libs ImageMagick++` \
        @LIBFFMPEG@ @PANGOFT2_LIBS@ 
avg_la_LIBADD = ../conradrelais/libconradrelais.a \
                ../player/libplayer.a \
				../graphics/libgraphics.a \
				../base/libbase.a \
				../parport/libparport.a
pkgpyexec_PYTHON = anim.py


