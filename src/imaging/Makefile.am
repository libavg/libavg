
INCLUDES = -I.. @PTHREAD_CFLAGS@

ALL_H = ConnectedComps.h Camera.h CameraThread.h CameraUtils.h
ALL_CPP = ConnectedComps.cpp Camera.cpp CameraThread.cpp CameraUtils.cpp

#TESTS=testcomps

EXTRA_DIST = test_comp.png

noinst_LTLIBRARIES = libimaging.la
libimaging_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testimaging
testimaging_SOURCES=testcomps.cpp $(ALL_H)
testimaging_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
testimaging_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@
