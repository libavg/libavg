
INCLUDES = -I.. $(MAGICK_CFLAGS) @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@  @SWSCALE_CFLAGS@
        
ALL_H = FFMpegDemuxer.h VideoDemuxerThread.h FFMpegDecoder.h IVideoDecoder.h \
        VideoDecoderThread.h AudioDecoderThread.h VideoMsg.h FrameVideoMsg.h \
        AudioVideoMsg.h EOFVideoMsg.h SeekDoneVideoMsg.h \
        ErrorVideoMsg.h PacketVideoMsg.h AsyncVideoDecoder.h VideoDecoderThread.h \
        IDemuxer.h AsyncDemuxer.h

TESTS=testvideo

EXTRA_DIST = $(wildcard testfiles/*.mov) $(wildcard testfiles/*.mpg) \
        $(wildcard testfiles/*.avi) $(wildcard testfiles/*.h264) \
		$(wildcard testfiles/baseline/*.png)

noinst_LTLIBRARIES = libvideo.la
noinst_PROGRAMS=testvideo

libvideo_la_SOURCES = FFMpegDemuxer.cpp VideoDemuxerThread.cpp FFMpegDecoder.cpp \
        VideoDecoderThread.cpp AudioDecoderThread.cpp VideoMsg.cpp FrameVideoMsg.cpp \
        AudioVideoMsg.cpp ErrorVideoMsg.cpp SeekDoneVideoMsg.cpp PacketVideoMsg.cpp \
        AsyncVideoDecoder.cpp AsyncDemuxer.cpp \
		$(ALL_H)

testvideo_SOURCES = testvideo.cpp $(ALL_H)
testvideo_LDADD = ./libvideo.la ../graphics/libgraphics.la ../base/libbase.la
testvideo_LDFLAGS = @XML2_LIBS@  -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ \
		@LIBFFMPEG@  @SWSCALE_LIBS@ $(MAGICK_LIBS)
