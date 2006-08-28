
INCLUDES = -I.. $(MAGICK_CFLAGS)
		
ALL_H = Bitmap.h Rect.h Point.h Filter.h\
		Pixel32.h Pixel24.h Pixel16.h Pixeldefs.h \
		Filtercolorize.h Filterfill.h Filterfillrect.h Filterflip.h \
		Filterfliprgb.h Filterflipuv.h Filtergrayscale.h Filter3x3.h
ALL_CPP = Bitmap.cpp Filter.cpp Filtergrayscale.cpp \
		Filtercolorize.cpp Filterflip.cpp Filterfliprgb.cpp \
        Filterflipuv.cpp Filter3x3.cpp

TESTS=testgraphics

EXTRA_DIST = rgb24-64x64.png

noinst_LTLIBRARIES = libgraphics.la
libgraphics_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testgraphics
testgraphics_SOURCES=testgraphics.cpp $(ALL_H)
testgraphics_LDADD = ./libgraphics.la ../base/libbase.la
testgraphics_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@
