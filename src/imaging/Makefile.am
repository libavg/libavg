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

ALL_H = Camera.h FWCamera.h \
        FakeCamera.h $(DC1394_INCLUDES) \
        $(V4L2_INCLUDES) CameraInfo.h
ALL_CPP = Camera.cpp FWCamera.cpp \
        FakeCamera.cpp $(DC1394_SOURCES) \
        $(V4L2_SOURCES) CameraInfo.cpp

EXTRA_DIST = $(wildcard baseline/*.png) $(wildcard testfiles/*.png) \
        CMUCamera.h CMUCamera.cpp DSCamera.cpp DSCamera.h DSHelper.h DSHelper.cpp \
        DSSampleGrabber.h DSSampleGrabber.cpp CMUCameraUtils.h CMUCameraUtils.cpp

noinst_LTLIBRARIES = libimaging.la
libimaging_la_SOURCES = $(ALL_CPP) $(ALL_H)
libimaging_la_LIBADD = @LIBIMAGING_LDADD@
