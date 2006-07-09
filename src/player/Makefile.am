
INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ @DIRECTFB_CFLAGS@ \
		@PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
		`pkg-config --cflags-only-I ImageMagick++`
        
if ENABLE_1394
    DC1394_LIBS = -ldc1394_control -lraw1394
else
    DC1394_LIBS =
endif

if APPLE
    APPLE_SOURCES = SDLMain.m AppleGLHelper.cpp
    APPLE_INCLUDES = AppleGLHelper.h
    XGL_LIBS =
else
    APPLE_SOURCES =
    APPLE_INCLUDES = 
    XGL_LIBS = -lXxf86vm
endif

if ENABLE_GL
    ALL_GL_LIBS = @SDL_LIBS@ $(XGL_LIBS)
    GL_SOURCES = OGLHelper.cpp OGLSurface.cpp SDLDisplayEngine.cpp OGLTile.cpp \
				 OGLShader.cpp
    GL_INCLUDES = SDLDisplayEngine.h OGLHelper.h OGLSurface.h OGLTile.h \
	              OGLShader.h GL/gl.h GL/glext.h GL/glu.h GL/glx.h
else
    ALL_GL_LIBS =
    GL_SOURCES =
    GL_INCLUDES =
endif

if ENABLE_DFB
    DFB_LIBS = @DIRECTFB_LIBS@
    DFB_SOURCES = DFBDisplayEngine.cpp DFBSurface.cpp 
    DFB_INCLUDES = DFBDisplayEngine.h DFBSurface.h 
else
    DFB_LIBS =
    DFB_SOURCES =
    DFB_INCLUDES =
endif

ALL_H = Player.h IEventSource.h IEventSink.h VideoBase.h Video.h\
        DisplayEngine.h Region.h Node.h \
        ISurface.h avgdtd.h\
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
		Player.cpp VideoBase.cpp Video.cpp DisplayEngine.cpp\
        Region.cpp Node.cpp FFMpegDecoder.cpp \
        MathHelper.cpp PanoImage.cpp Words.cpp Camera.cpp \
        DivNode.cpp Timeout.cpp DebugEventSink.cpp Event.cpp \
        Image.cpp EventDispatcher.cpp KeyEvent.cpp MouseEvent.cpp\
        RasterNode.cpp AVGNode.cpp avgdtd.cpp TestHelper.cpp\
        $(APPLE_SOURCES) \
		$(ALL_H)
libplayer_ladir=$(includedir)/avg/player
libplayer_la_LIBADD = ../graphics/libgraphics.la
libplayer_la_LDFLAGS = @XML2_LIBS@ \
        $(DC1394_LIBS) @GLU_LIBS@ $(ALL_GL_LIBS) $(DFB_LIBS) \
		`pkg-config --libs ImageMagick++` \
        @LIBFFMPEG@ @PANGOFT2_LIBS@ 
libplayer_la_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
