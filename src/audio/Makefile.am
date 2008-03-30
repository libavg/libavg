
INCLUDES = -I.. @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@ @SWSCALE_CFLAGS@
        
ALL_H = AudioEngine.h AudioFrame.h AudioParams.h AudioSource.h SDLAudioEngine.h 

#TESTS=testaudio

#EXTRA_DIST = testfiles/mjpeg-48x48.avi testfiles/mpeg1-48x48.mpg \
#		$(wildcard testfiles/baseline/*.png)

noinst_LTLIBRARIES = libaudio.la
#noinst_PROGRAMS=testaudio

libaudio_la_SOURCES = AudioEngine.cpp AudioFrame.cpp AudioParams.cpp \
        SDLAudioEngine.cpp \
		$(ALL_H)

#testaudio_SOURCES = testaudio.cpp $(ALL_H)
#testaudio_LDADD = ./libaudio.la ../graphics/libgraphics.la ../base/libbase.la
#testaudio_LDFLAGS = @XML2_LIBS@  -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ \
#		@LIBFFMPEG@  @SWSCALE_LIBS@ $(MAGICK_LIBS)
