
INCLUDES = -I.  \
        `xml2-config --cflags` \
        $(PYTHON_CSPEC)

ALL_H = ConradRelais.h

noinst_LTLIBRARIES = libconradrelais.la
libconradrelais_la_SOURCES = ConradRelais.cpp $(ALL_H)
libconradrelais_la_HEADERS = $(ALL_H)
libconradrelais_ladir=$(includedir)/avg/conradrelais
