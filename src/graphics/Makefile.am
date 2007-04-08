
INCLUDES = -I.. $(MAGICK_CFLAGS)
		
ALL_H = Bitmap.h Rect.h Point.h Filter.h\
		Pixel32.h Pixel24.h Pixel16.h Pixeldefs.h \
		Filtercolorize.h Filterfill.h Filterfillrect.h Filterflip.h \
		Filterfliprgb.h Filterflipuv.h Filtergrayscale.h Filter3x3.h \
		FilterId.h HistoryPreProcessor.h FilterConvol.h FilterHighpass.h \
        FilterFastBandpass.h \
        FilterGauss.h FilterBandpass.h FilterBlur.h
ALL_CPP = Bitmap.cpp Point.cpp Filter.cpp Filtergrayscale.cpp \
		Filtercolorize.cpp Filterflip.cpp Filterfliprgb.cpp \
        Filterflipuv.cpp Filter3x3.cpp HistoryPreProcessor.cpp FilterHighpass.cpp \
        FilterFastBandpass.cpp \
        FilterGauss.cpp FilterBandpass.cpp FilterBlur.cpp  

TESTS=testgraphics

EXTRA_DIST = rgb24-64x64.png

noinst_LTLIBRARIES = libgraphics.la
libgraphics_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testgraphics benchmarkgraphics
testgraphics_SOURCES=testgraphics.cpp $(ALL_H)
testgraphics_LDADD = ./libgraphics.la ../base/libbase.la
testgraphics_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@

benchmarkgraphics_SOURCES=benchmarkgraphics.cpp $(ALL_H)
benchmarkgraphics_LDADD = ./libgraphics.la ../base/libbase.la
benchmarkgraphics_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@
