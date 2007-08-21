
INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ \
		@PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
        @DC1394_2_CFLAGS@ \
		$(MAGICK_CFLAGS)
        
if APPLE
    APPLE_SOURCES = SDLMain.m AppleGLHelper.cpp
    APPLE_INCLUDES = AppleGLHelper.h
else
    APPLE_SOURCES =
    APPLE_INCLUDES = 
endif

if ENABLE_GL
    GL_SOURCES = OGLHelper.cpp OGLSurface.cpp SDLDisplayEngine.cpp OGLTile.cpp \
				 OGLShader.cpp
    GL_INCLUDES = SDLDisplayEngine.h OGLHelper.h OGLSurface.h OGLTile.h \
	              OGLShader.h GL/gl.h GL/glext.h GL/glu.h GL/glx.h
else
    GL_SOURCES =
    GL_INCLUDES =
endif

ALL_H = Player.h IEventSource.h IEventSink.h VideoBase.h Video.h\
        DisplayEngine.h Region.h Node.h \
        ISurface.h avgdtd.h DisplayParams.h \
        MathHelper.h AVGNode.h DivNode.h \
        Image.h Timeout.h Words.h \
        DebugEventSink.h EventDispatcher.h CursorEvent.h MouseEvent.h \
        Event.h KeyEvent.h PanoImage.h TestHelper.h \
        RasterNode.h CameraNode.h TrackerEventSource.h TrackerCalibrator.h\
	    TouchEvent.h EventStream.h\
        $(GL_INCLUDES) $(APPLE_INCLUDES)

TESTS=testcalibrator

noinst_LTLIBRARIES = libplayer.la
noinst_PROGRAMS = checktracker testcalibrator
checktracker_SOURCES=checktracker.cpp
checktracker_LDADD = libplayer.la ../video/libvideo.la ../imaging/libimaging.la \
		../graphics/libgraphics.la ../base/libbase.la ../lmfit/liblmfit.la
checktracker_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ 

testcalibrator_SOURCES=testcalibrator.cpp
testcalibrator_LDADD = libplayer.la ../video/libvideo.la ../imaging/libimaging.la \
		../graphics/libgraphics.la ../base/libbase.la ../lmfit/liblmfit.la
testcalibrator_LDFLAGS = $(MAGICK_LIBS) @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ 

libplayer_la_SOURCES = $(GL_SOURCES) \
		Node.cpp RasterNode.cpp DivNode.cpp VideoBase.cpp Video.cpp \
        Player.cpp DisplayEngine.cpp Region.cpp \
        MathHelper.cpp PanoImage.cpp Words.cpp CameraNode.cpp \
        Timeout.cpp DebugEventSink.cpp Event.cpp DisplayParams.cpp \
        Image.cpp EventDispatcher.cpp KeyEvent.cpp CursorEvent.cpp MouseEvent.cpp\
        TouchEvent.cpp AVGNode.cpp avgdtd.cpp TestHelper.cpp \
		TrackerEventSource.cpp EventStream.cpp TrackerCalibrator.cpp \
        $(APPLE_SOURCES) \
		$(ALL_H)
libplayer_a_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
