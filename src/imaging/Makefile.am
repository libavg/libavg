
INCLUDES = -I.. @PTHREAD_CFLAGS@  @XML2_CFLAGS@

if ENABLE_1394
    DC1394_SOURCES = CameraUtils.cpp
    DC1394_INCLUDES = CameraUtils.h
else
if ENABLE_1394_2
    DC1394_SOURCES = CameraUtils.cpp
    DC1394_INCLUDES = CameraUtils.h
else

    DC1394_SOURCES =
    DC1394_INCLUDES =
endif
endif

ALL_H = TrackerThread.h TrackerConfig.h ConnectedComps.h Camera.h \
        FakeCamera.h CoordTransformer.h FilterDistortion.h $(DC1394_INCLUDES) \
        BlobInfo.h DeDistort.h
ALL_CPP = TrackerThread.cpp TrackerConfig.cpp ConnectedComps.cpp Camera.cpp \
        FakeCamera.cpp CoordTransformer.cpp FilterDistortion.cpp $(DC1394_SOURCES) \
        DeDistort.cpp

TESTS=testimaging

EXTRA_DIST = test_comp.png

noinst_LTLIBRARIES = libimaging.la
libimaging_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testimaging checkcomps checktracking

testimaging_SOURCES=testimaging.cpp $(ALL_H)
testimaging_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
testimaging_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@

checkcomps_SOURCES=checkcomps.cpp $(ALL_H)
checkcomps_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
checkcomps_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@

checktracking_SOURCES=checktracking.cpp $(ALL_H)
checktracking_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la
checktracking_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@
