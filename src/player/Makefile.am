
INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ \
		@PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
        @DC1394_2_CFLAGS@ \
		$(MAGICK_CFLAGS)
        
if ENABLE_1394
    DC1394_LIBS = -ldc1394_control -lraw1394
else
    DC1394_LIBS =
endif

if APPLE
    APPLE_SOURCES = SDLMain.m AppleGLHelper.cpp
    APPLE_INCLUDES = AppleGLHelper.h
    XGL_LIBS =
    APPLE_LINKFLAGS=-read_only_relocs suppress
else
    APPLE_SOURCES =
    APPLE_INCLUDES = 
    XGL_LIBS = -lXxf86vm
    APPLE_LINKFLAGS=
endif

GL_SOURCES = OGLHelper.cpp OGLSurface.cpp SDLDisplayEngine.cpp OGLTexture.cpp \
        	 OGLShader.cpp VertexArray.cpp
GL_INCLUDES = SDLDisplayEngine.h OGLHelper.h OGLSurface.h OGLTexture.h \
              OGLShader.h VertexArray.h GL/gl.h GL/glext.h GL/glu.h GL/glx.h

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

ALL_H = Player.h IEventSource.h IEventSink.h VideoBase.h Video.h\
        DisplayEngine.h NodeFactory.h Arg.h ArgList.h Region.h Node.h \
        ISurface.h DisplayParams.h NodeDefinition.h \
        MathHelper.h AVGNode.h DivNode.h \
        Image.h Timeout.h Words.h WrapPython.h\
        EventDispatcher.h CursorEvent.h MouseEvent.h \
        Event.h KeyEvent.h PanoImage.h TestHelper.h \
        RasterNode.h CameraNode.h TrackerEventSource.h TrackerCalibrator.h\
	    TouchEvent.h EventStream.h BoostPython.h \
        $(GL_INCLUDES) $(APPLE_INCLUDES)

TESTS=testcalibrator testplayer

noinst_LTLIBRARIES = libplayer.la
noinst_PROGRAMS = checktracker testcalibrator testplayer
checktracker_SOURCES=checktracker.cpp
checktracker_LDADD = libplayer.la ../video/libvideo.la ../imaging/libimaging.la \
		../graphics/libgraphics.la ../base/libbase.la ../lmfit/liblmfit.la
checktracker_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(MAGICK_LIBS) \
		$(BOOST_PYTHON_LIBS) $(PYTHON_LIBS)

testplayer_SOURCES=testplayer.cpp
testplayer_LDADD = libplayer.la ../video/libvideo.la ../imaging/libimaging.la \
		../graphics/libgraphics.la ../base/libbase.la ../lmfit/liblmfit.la
testplayer_LDFLAGS = $(APPLE_LINKFLAGS) -module -XCClinker  \
        @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(DC1394_LIBS) \
        @DC1394_2_LIBS@ @GLU_LIBS@ $(ALL_GL_LIBS) @LIBFFMPEG@ @SWSCALE_LIBS@ \
        @PANGOFT2_LIBS@ $(BOOST_PYTHON_LIBS) $(PYTHON_LIBS) $(MAGICK_LIBS) 

testcalibrator_SOURCES=testcalibrator.cpp
testcalibrator_LDADD = libplayer.la ../video/libvideo.la ../imaging/libimaging.la \
		../graphics/libgraphics.la ../base/libbase.la ../lmfit/liblmfit.la
testcalibrator_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(MAGICK_LIBS)

libplayer_la_LDFLAGS = $(BOOST_PYTHON_LIBS) $(PYTHON_LIBS)
libplayer_la_SOURCES = $(GL_SOURCES) \
		Node.cpp RasterNode.cpp DivNode.cpp VideoBase.cpp Video.cpp \
        Player.cpp NodeFactory.cpp Arg.cpp ArgList.cpp DisplayEngine.cpp Region.cpp \
        MathHelper.cpp PanoImage.cpp Words.cpp CameraNode.cpp NodeDefinition.cpp \
        Timeout.cpp Event.cpp DisplayParams.cpp \
        Image.cpp EventDispatcher.cpp KeyEvent.cpp CursorEvent.cpp MouseEvent.cpp\
        TouchEvent.cpp AVGNode.cpp TestHelper.cpp \
		TrackerEventSource.cpp EventStream.cpp TrackerCalibrator.cpp \
        $(APPLE_SOURCES) \
		$(ALL_H)
libplayer_a_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
