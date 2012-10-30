
INCLUDES = -I.. @XML2_CFLAGS@ @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@ @GDK_PIXBUF_CFLAGS@
        
if APPLE
    X_LIBS = 
else
    X_LIBS = -lX11
endif

ALL_H = FFMpegDemuxer.h VideoDemuxerThread.h FFMpegDecoder.h VideoDecoder.h \
        VideoDecoderThread.h AudioDecoderThread.h VideoMsg.h \
        PacketVideoMsg.h AsyncVideoDecoder.h VideoDecoderThread.h \
        IDemuxer.h AsyncDemuxer.h VideoInfo.h WrapFFMpeg.h

if USE_VDPAU_SRC
        ALL_H += VDPAUDecoder.h VDPAUHelper.h
endif

TESTS=testvideo

EXTRA_DIST = $(wildcard baseline/*.png)

noinst_LTLIBRARIES = libvideo.la
noinst_PROGRAMS=testvideo

libvideo_la_SOURCES = FFMpegDemuxer.cpp VideoDemuxerThread.cpp FFMpegDecoder.cpp \
        VideoDecoderThread.cpp AudioDecoderThread.cpp VideoMsg.cpp VideoDecoder.cpp \
        PacketVideoMsg.cpp AsyncVideoDecoder.cpp AsyncDemuxer.cpp VideoInfo.cpp \
        $(ALL_H)

if USE_VDPAU_SRC
        libvideo_la_SOURCES += VDPAUDecoder.cpp VDPAUHelper.cpp
endif

libvideo_la_LIBADD = @LIBVIDEO_LDADD@

testvideo_SOURCES = testvideo.cpp $(ALL_H)
testvideo_LDADD = ./libvideo.la ../audio/libaudio.la ../graphics/libgraphics.la \
        ../base/libbase.la ../base/triangulate/libtriangulate.la \
        @XML2_LIBS@  @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@ \
        @LIBFFMPEG@ $(X_LIBS) \
        @GDK_PIXBUF_LIBS@

