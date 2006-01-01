
INCLUDES = -I.. \
		`Magick++-config --cppflags` 
        
ALL_H = Bitmap.h Rect.h Point.h Filter.h\
		Pixel32.h Pixel24.h Pixel16.h Pixel8.h Pixeldefs.h \
		Filtercolorize.h Filterfill.h Filterfillrect.h Filterflip.h \
		Filterfliprgb.h Filtergrayscale.h 
ALL_CPP = Bitmap.cpp Filter.cpp Filtergrayscale.cpp \
		Filtercolorize.cpp Filterflip.cpp Filterfliprgb.cpp

noinst_LTLIBRARIES = libgraphics.la
libgraphics_la_SOURCES = $(ALL_CPP) $(ALL_H)
libgraphics_ladir=$(includedir)/avg/graphics
libgraphics_la_LIBADD = ../base/libbase.la        
libgraphics_la_LDFLAGS = `Magick++-config --ldflags --libs`
noinst_PROGRAMS=testgraphics
testgraphics_SOURCES=testgraphics.cpp $(ALL_H)
testgraphics_LDADD = ../base/libbase.la ./libgraphics.la
