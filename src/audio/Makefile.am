
INCLUDES = -I.. @PTHREAD_CFLAGS@ @FFMPEG_CFLAGS@ @SWSCALE_CFLAGS@

ALL_H = AudioEngine.h AudioBuffer.h AudioParams.h IAudioSource.h SDLAudioEngine.h \
        Dynamics.h IProcessor.h

TESTS=testlimiter

noinst_LTLIBRARIES = libaudio.la
noinst_PROGRAMS=testlimiter

libaudio_la_SOURCES = AudioEngine.cpp AudioBuffer.cpp AudioParams.cpp \
        SDLAudioEngine.cpp \
        $(ALL_H)

testlimiter_SOURCES = testlimiter.cpp $(ALL_H)
testlimiter_LDADD = ./libaudio.la ../base/libbase.la
testlimiter_LDFLAGS = 
