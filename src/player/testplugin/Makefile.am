
INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ \
        @PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
        @DC1394_2_CFLAGS@ \
        $(MAGICK_CFLAGS)

ALL_H =

noinst_LTLIBRARIES = libColorNode.la

## TODO: make the plugin build as a bundle on Darwin
## THe LIBTOOLFLAG -module is supposed to do that. DOesn't work
# for me however. Also saw an -export-dynamic in the wild
libColorNode_la_LIBTOOLFLAGS = 
libColorNode_la_LDFLAGS = 
libColorNode_la_SOURCES = ColorNode.cpp $(ALL_H)
libColorNode_la_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
