
INCLUDES = -I.. @PTHREAD_CFLAGS@

if ENABLE_1394
    DC1394_SOURCES = CameraThread.cpp CameraUtils.cpp CameraCmd.cpp
    DC1394_INCLUDES = CameraThread.h CameraUtils.h CameraCmd.h
else
if ENABLE_1394_2
    DC1394_SOURCES = CameraThread.cpp CameraUtils.cpp CameraCmd.cpp
    DC1394_INCLUDES = CameraThread.h CameraUtils.h CameraCmd.h
else

    DC1394_SOURCES =
    DC1394_INCLUDES =
endif
endif

ALL_H = TrackerThread.h TrackerCmd.h ConnectedComps.h Camera.h \
        FakeCamera.h $(DC1394_INCLUDES)
ALL_CPP = TrackerThread.cpp TrackerCmd.cpp ConnectedComps.cpp Camera.cpp \
        FakeCamera.cpp $(DC1394_SOURCES) 

TESTS=testimaging

EXTRA_DIST = test_comp.png

noinst_LTLIBRARIES = libimaging.la
libimaging_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testimaging testcomps
testimaging_SOURCES=testimaging.cpp $(ALL_H)
testimaging_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
testimaging_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@

testcomps_SOURCES=testcomps.cpp $(ALL_H)
testcomps_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
testcomps_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@
