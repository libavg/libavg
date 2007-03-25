
INCLUDES = -I.. @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@ 
        
ALL_H = FFMpegDemuxer.h VideoDemuxerThread.h FFMpegDecoder.h IVideoDecoder.h \
        VideoDecoderThread.h IVideoMsg.h FrameVideoMsg.h InfoVideoMsg.h \
        EOFVideoMsg.h ErrorVideoMsg.h PacketVideoMsg.h \
	    AsyncVideoDecoder.h VideoDecoderThread.h IDemuxer.h AsyncDemuxer.h

noinst_LTLIBRARIES = libvideo.la

libvideo_la_SOURCES =  FFMpegDemuxer.cpp VideoDemuxerThread.cpp FFMpegDecoder.cpp \
        VideoDecoderThread.cpp FrameVideoMsg.cpp InfoVideoMsg.cpp ErrorVideoMsg.cpp \
        PacketVideoMsg.cpp AsyncVideoDecoder.cpp AsyncDemuxer.cpp \
		$(ALL_H)
