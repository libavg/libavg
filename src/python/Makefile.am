
INCLUDES = -I. -I../player \
		$(XML2_CFLAGS) $(PANGOFT2_CFLAGS) $(PYTHON_CSPEC)
		
ALL_H = 

if ENABLE_PARPORT
    PARPORT_LIB = ../parport/libparport.la
else
    PARPORT_LIB =
endif

pkgpyexec_LTLIBRARIES = avg.la
avg_la_SOURCES = avg_wrap.cpp raster_wrap.cpp event_wrap.cpp device_wrap.cpp
avg_la_LDFLAGS = -module $(PYTHON_LSPEC) -lboost_python 
avg_la_LIBADD = ../conradrelais/libconradrelais.la \
                $(PARPORT_LIB) $(LIBFFMPEG)\
                ../player/libplayer.la 
pkgpyexec_PYTHON = anim.py

