
AM_CPPFLAGS = -I. -I../player \
		@XML2_CFLAGS@ @PTHREAD_CFLAGS@ @PANGOFT2_CFLAGS@ @PYTHON_CPPFLAGS@ @DC1394_2_CFLAGS@

ALL_H =

if APPLE
    XGL_LIBS =
    EXTRA_LDFLAGS=-read_only_relocs suppress
else
    XGL_LIBS = -lXxf86vm
    EXTRA_LDFLAGS=-XCClinker ../../wrapper/.libs/avg.so
endif

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

pkgpyexec_LTLIBRARIES = colorplugin.la
colorplugin_la_SOURCES = ColorNode.cpp 
colorplugin_la_LDFLAGS = $(EXTRA_LDFLAGS) -module 
