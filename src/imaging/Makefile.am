AM_CPPFLAGS = -I.. @PTHREAD_CFLAGS@ @XML2_CFLAGS@ @GDK_PIXBUF_CFLAGS@

if ENABLE_V4L2
   V4L2_SOURCES = V4LCamera.cpp
   V4L2_INCLUDES = V4LCamera.h
else
   V4L2_SOURCES =
   V4L2_INCLUDES =
endif

if ENABLE_1394_2
    DC1394_SOURCES = FWCameraUtils.cpp
    DC1394_INCLUDES = FWCameraUtils.h
else
    DC1394_SOURCES =
    DC1394_INCLUDES =
endif

ALL_H = Camera.h TrackerThread.h TrackerConfig.h Blob.h FWCamera.h Run.h \
        FakeCamera.h CoordTransformer.h FilterDistortion.h $(DC1394_INCLUDES) \
        DeDistort.h trackerconfigdtd.h  FilterWipeBorder.h FilterClearBorder.h \
        $(V4L2_INCLUDES) CameraInfo.h
ALL_CPP = Camera.cpp TrackerThread.cpp TrackerConfig.cpp Blob.cpp FWCamera.cpp Run.cpp \
        FakeCamera.cpp CoordTransformer.cpp FilterDistortion.cpp $(DC1394_SOURCES) \
        DeDistort.cpp trackerconfigdtd.cpp FilterWipeBorder.cpp FilterClearBorder.cpp \
        $(V4L2_SOURCES) CameraInfo.cpp

TESTS = testimaging

EXTRA_DIST = avgtrackerrc.minimal $(wildcard baseline/*.png) $(wildcard testfiles/*.png) \
        CMUCamera.h CMUCamera.cpp DSCamera.cpp DSCamera.h DSHelper.h DSHelper.cpp \
        DSSampleGrabber.h DSSampleGrabber.cpp CMUCameraUtils.h CMUCameraUtils.cpp

noinst_LTLIBRARIES = libimaging.la
libimaging_la_SOURCES = $(ALL_CPP) $(ALL_H)

noinst_PROGRAMS = testimaging
testimaging_SOURCES = testimaging.cpp $(ALL_H)
testimaging_LDADD = ./libimaging.la ../graphics/libgraphics.la ../base/libbase.la \
        ../base/triangulate/libtriangulate.la \
        @XML2_LIBS@ @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@ @GDK_PIXBUF_LIBS@
