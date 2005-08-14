
INCLUDES = -I. \
		`Magick++-config --cppflags` 
        
ALL_H = Bitmap.h Rect.h Point.h \
		Pixel32.h Pixel24.h Pixel16.h Pixel8.h Pixeldefs.h

lib_LTLIBRARIES = libgraphics.la
libgraphics_la_SOURCES = Bitmap.cpp $(ALL_H)
libgraphics_ladir=$(includedir)/avg/graphics
libgraphics_la_HEADERS = $(ALL_H)
libgraphics_la_LIBADD = ../base/libbase.la        
libgraphics_la_LDFLAGS = `Magick++-config --libs` -lboost_python
