AM_CPPFLAGS = -I.. @XML2_CFLAGS@ @PTHREAD_CFLAGS@ @GDK_PIXBUF_CFLAGS@ @FFMPEG_CFLAGS@

if APPLE
    X_LIBS =
else
if ENABLE_RPI
    X_LIBS = -lX11 -lbcm_host -lGLESv2 -lEGL
else
if ENABLE_EGL
    X_LIBS = -lXxf86vm -lX11 -lGLESv2 -lEGL
else
    X_LIBS = -lXxf86vm -lX11
endif
endif
endif

ALL_H = FFMpegDemuxer.h VideoDemuxerThread.h VideoDecoder.h \
        VideoDecoderThread.h AudioDecoderThread.h VideoMsg.h FFMpegFrameDecoder.h \
        AsyncVideoDecoder.h VideoDecoderThread.h SyncVideoDecoder.h \
        VideoInfo.h WrapFFMpeg.h

TESTS = testvideo

EXTRA_DIST = $(wildcard baseline/*.png)

noinst_LTLIBRARIES = libvideo.la
noinst_PROGRAMS = testvideo

libvideo_la_SOURCES = FFMpegDemuxer.cpp VideoDemuxerThread.cpp VideoDecoder.cpp \
        VideoDecoderThread.cpp AudioDecoderThread.cpp VideoMsg.cpp \
        AsyncVideoDecoder.cpp VideoInfo.cpp SyncVideoDecoder.cpp \
        FFMpegFrameDecoder.cpp WrapFFMpeg.cpp \
        $(ALL_H)

libvideo_la_LIBADD = @LIBVIDEO_LDADD@

testvideo_SOURCES = testvideo.cpp $(ALL_H)
testvideo_LDADD = ./libvideo.la ../audio/libaudio.la ../graphics/libgraphics.la \
        ../base/libbase.la -ldl \
        @SDL_LIBS@ @XML2_LIBS@ \
        @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@ @LIBFFMPEG@ @LIBAVRESAMPLE@ @GDK_PIXBUF_LIBS@ \
        $(X_LIBS)
