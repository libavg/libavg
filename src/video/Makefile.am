
INCLUDES = -I.. @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@ 
        
ALL_H = FFMpegDemuxer.h VideoDemuxerThread.h FFMpegDecoder.h IVideoDecoder.h \
        VideoDecoderThread.h IVideoMsg.h FrameVideoMsg.h InfoVideoMsg.h \
        EOFVideoMsg.h ErrorVideoMsg.h PacketVideoMsg.h \
	    AsyncVideoDecoder.h VideoDecoderThread.h IDemuxer.h AsyncDemuxer.h

TESTS=testvideo

noinst_LTLIBRARIES = libvideo.la
noinst_PROGRAMS=testvideo

libvideo_la_SOURCES = FFMpegDemuxer.cpp VideoDemuxerThread.cpp FFMpegDecoder.cpp \
        VideoDecoderThread.cpp FrameVideoMsg.cpp InfoVideoMsg.cpp ErrorVideoMsg.cpp \
        PacketVideoMsg.cpp AsyncVideoDecoder.cpp AsyncDemuxer.cpp \
		$(ALL_H)

testvideo_SOURCES = testvideo.cpp $(ALL_H)
testvideo_LDADD = ./libvideo.la ../graphics/libgraphics.la ../base/libbase.la
testvideo_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@  -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ @LIBFFMPEG@ 
