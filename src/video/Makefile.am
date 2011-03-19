
INCLUDES = -I.. $(MAGICK_CFLAGS) @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@
        
if APPLE
    X_LIBS = 
else
    X_LIBS = -lX11
endif

ALL_H = FFMpegDemuxer.h VideoDemuxerThread.h FFMpegDecoder.h IVideoDecoder.h \
        VideoDecoderThread.h AudioDecoderThread.h VideoMsg.h \
        PacketVideoMsg.h AsyncVideoDecoder.h VideoDecoderThread.h \
        IDemuxer.h AsyncDemuxer.h VideoInfo.h WrapFFMpeg.h VDPAU.h

TESTS=testvideo

EXTRA_DIST = $(wildcard testfiles/*.mov) $(wildcard testfiles/*.mpg) \
        $(wildcard testfiles/*.avi) $(wildcard testfiles/*.h264) \
        $(wildcard testfiles/*.wav) $(wildcard testfiles/*.ogg) \
        $(wildcard testfiles/*.mp3) $(wildcard testfiles/*.aif) \
        $(wildcard testfiles/*.flv) $(wildcard baseline/*.png)

noinst_LTLIBRARIES = libvideo.la
noinst_PROGRAMS=testvideo

libvideo_la_SOURCES = FFMpegDemuxer.cpp VideoDemuxerThread.cpp FFMpegDecoder.cpp \
        VideoDecoderThread.cpp AudioDecoderThread.cpp VideoMsg.cpp \
        PacketVideoMsg.cpp AsyncVideoDecoder.cpp AsyncDemuxer.cpp VideoInfo.cpp \
        VDPAU.cpp \
        $(ALL_H)

libvideo_la_LIBADD = @LIBVIDEO_LDADD@

testvideo_SOURCES = testvideo.cpp $(ALL_H)
testvideo_LDADD = ./libvideo.la ../audio/libaudio.la ../graphics/libgraphics.la \
        ../base/libbase.la \
        @XML2_LIBS@  -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ \
        @LIBFFMPEG@ $(MAGICK_LIBS) $(X_LIBS)
