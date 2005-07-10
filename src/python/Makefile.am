
INCLUDES = -I. \
        `xml2-config --cflags` \
        `pkg-config --cflags pango` \
        -I/usr/include/python2.3
		
ALL_H = 

pkgpyexec_LTLIBRARIES = avg.la
avg_la_SOURCES = avg_wrap.cpp raster_wrap.cpp event_wrap.cpp
avg_la_LDFLAGS = -module -lboost_python
avg_la_LIBADD = ../base/libbase.la ../conradrelais/libconradrelais.la \
                ../player/libplayer.la 

# TODO: Remove -fpermissive
AM_CXXFLAGS = -fpermissive $(CXXFLAGS)


