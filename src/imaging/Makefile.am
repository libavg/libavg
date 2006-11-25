
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

ALL_H = Tracker.h TrackerThread.h ConnectedComps.h Camera.h $(DC1394_INCLUDES)
ALL_CPP = Tracker.cpp TrackerThread.cpp ConnectedComps.cpp Camera.cpp $(DC1394_SOURCES) 

#TESTS=testcomps

EXTRA_DIST = test_comp.png

noinst_LTLIBRARIES = libimaging.la
libimaging_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testimaging
testimaging_SOURCES=testcomps.cpp $(ALL_H)
testimaging_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
testimaging_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@
