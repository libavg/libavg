
INCLUDES = -I. -I../player \
		@XML2_CFLAGS@ @PTHREAD_CFLAGS@ @PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @DC1394_2_CFLAGS@

ALL_H =

if ENABLE_1394
    DC1394_LIBS = -ldc1394_control -lraw1394
else
    DC1394_LIBS =
endif

if APPLE
    XGL_LIBS =
    APPLE_LDFLAGS=-read_only_relocs suppress
else
    XGL_LIBS = -lXxf86vm
    APPLE_LDFLAGS=
endif

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

lib_LTLIBRARIES = libColorNode.la
libColorNode_la_SOURCES = ColorNode.cpp 
libColorNode_la_LDFLAGS = $(APPLE_LDFLAGS) -module -XCClinker ../../wrapper/.libs/avg.so

