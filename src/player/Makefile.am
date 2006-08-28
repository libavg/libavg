
INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ @DIRECTFB_CFLAGS@ \
		@PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
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

if ENABLE_DFB
    DFB_SOURCES = DFBDisplayEngine.cpp DFBSurface.cpp 
    DFB_INCLUDES = DFBDisplayEngine.h DFBSurface.h 
else
    DFB_SOURCES =
    DFB_INCLUDES =
endif

ALL_H = Player.h IEventSource.h IEventSink.h VideoBase.h Video.h\
        DisplayEngine.h Region.h Node.h \
        ISurface.h avgdtd.h DisplayParams.h \
        MathHelper.h AVGNode.h DivNode.h \
        Image.h \
        Timeout.h Words.h \
        DebugEventSink.h EventDispatcher.h MouseEvent.h \
        Event.h KeyEvent.h \
        VideoBase.h \
        Video.h FFMpegDecoder.h IVideoDecoder.h \
        PanoImage.h TestHelper.h \
        RasterNode.h Camera.h \
        $(GL_INCLUDES) $(DFB_INCLUDES) $(APPLE_INCLUDES)

noinst_LTLIBRARIES = libplayer.la
libplayer_la_SOURCES = $(GL_SOURCES) $(DFB_SOURCES) \
		Node.cpp RasterNode.cpp DivNode.cpp VideoBase.cpp Video.cpp \
        Player.cpp DisplayEngine.cpp Region.cpp FFMpegDecoder.cpp \
        MathHelper.cpp PanoImage.cpp Words.cpp Camera.cpp \
        Timeout.cpp DebugEventSink.cpp Event.cpp DisplayParams.cpp \
        Image.cpp EventDispatcher.cpp KeyEvent.cpp MouseEvent.cpp\
        AVGNode.cpp avgdtd.cpp TestHelper.cpp\
        $(APPLE_SOURCES) \
		$(ALL_H)
libplayer_a_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
