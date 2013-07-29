INCLUDES = -I. @XML2_CFLAGS@ @PYTHON_CPPFLAGS@

if APPLE
    XGL_LIBS =
    EXTRA_LDFLAGS=-read_only_relocs suppress
else
    XGL_LIBS = -lXxf86vm
    EXTRA_LDFLAGS=-XCClinker ../../../wrapper/.libs/avg.so
endif

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

pkgplugindir = $(pkgpyexecdir)/samples/firebirds/plugin
pkgplugin_LTLIBRARIES = collisiondetector.la
collisiondetector_la_SOURCES = CollisionDetector.h CollisionDetector.cpp
collisiondetector_la_LDFLAGS = $(EXTRA_LDFLAGS) -module
