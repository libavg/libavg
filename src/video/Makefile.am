
INCLUDES = -I.. @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@ 
        
ALL_H = FFMpegDecoder.h IVideoDecoder.h VideoDecoderThread.h \
        IVideoMsg.h FrameVideoMsg.h InfoVideoMsg.h EOFVideoMsg.h ErrorVideoMsg.h \
	    AsyncVideoDecoder.h VideoDecoderThread.h

noinst_LTLIBRARIES = libvideo.la

libvideo_la_SOURCES =  FFMpegDecoder.cpp VideoDecoderThread.cpp \
		FrameVideoMsg.cpp InfoVideoMsg.cpp ErrorVideoMsg.cpp AsyncVideoDecoder.cpp\
		$(ALL_H)
