
INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ \
        @PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
        @DC1394_2_CFLAGS@ \
        $(MAGICK_CFLAGS)

ALL_H =

noinst_LTLIBRARIES = libColorNode.la

libColorNode_la_LIBTOOLFLAGS = -module -export-dynamic
libColorNode_la_LDFLAGS = -module
libColorNode_la_SOURCES = ColorNode.cpp $(ALL_H)
libColorNode_la_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
