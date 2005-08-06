
INCLUDES = -I. \
        `xml2-config --cflags` \
        `paintlib-config --cflags` \
        `freetype-config --cflags` \
        `directfb-config --cflags` \
        `pkg-config --cflags pango` \
        -I/usr/include/python2.4 \
$(GL_CFLAGS)
        
if ENABLE_1394
    DC1394_LIBS = -ldc1394_control -lraw1394
else
    DC1394_LIBS =
endif

if ENABLE_GL
    SDL_LIBS = `sdl-config --libs`
    GL_SOURCES = OGLHelper.cpp OGLSurface.cpp SDLDisplayEngine.cpp VBlank.cpp
    GL_INCLUDES = SDLDisplayEngine.h OGLHelper.h OGLSurface.h VBlank.h
else
    SDL_LIBS =
    GL_SOURCES =
    GL_INCLUDES =
endif

if ENABLE_DFB
    DFB_LIBS = `directfb-config --libs`
    DFB_SOURCES = DFBDisplayEngine.cpp DFBSurface.cpp 
    DFB_INCLUDES = DFBDisplayEngine.h DFBSurface.h 
else
    DFB_LIBS =
    DFB_SOURCES =
    DFB_INCLUDES =
endif
            
ALL_H = Player.h IEventSource.h IEventSink.h Excl.h VideoBase.h Video.h\
        IDisplayEngine.h Rect.h Point.h Region.h Node.h \
        ISurface.h FramerateManager.h Container.h \
        MathHelper.h AVGNode.h DivNode.h \
        Image.h \
        Timeout.h Words.h \
        DebugEventSink.h EventDispatcher.h MouseEvent.h \
        Event.h KeyEvent.h \
        VideoBase.h \
        Video.h FFMpegDecoder.h IVideoDecoder.h \
        Excl.h PanoImage.h \
        RasterNode.h Camera.h \
        $(GL_INCLUDES) $(DFB_INCLUDES)

avgdir = $(libdir)/avg
lib_LTLIBRARIES = libplayer.la
libplayer_la_SOURCES = Player.cpp Excl.cpp VideoBase.cpp Video.cpp \
        Region.cpp Node.cpp FramerateManager.cpp FFMpegDecoder.cpp \
        Container.cpp  MathHelper.cpp PanoImage.cpp Words.cpp Camera.cpp \
        DivNode.cpp Timeout.cpp DebugEventSink.cpp Event.cpp \
        Image.cpp EventDispatcher.cpp KeyEvent.cpp MouseEvent.cpp\
        RasterNode.cpp AVGNode.cpp $(GL_SOURCES) $(DFB_SOURCES) \
		$(ALL_H)
libplayer_ladir=$(includedir)/avg/player
libplayer_la_HEADERS = $(ALL_H)
libplayer_la_LIBADD = ../base/libbase.la        
libplayer_la_LDFLAGS = `xml2-config --libs` `paintlib-config --libs` \
        $(DC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(SDL_LIBS) $(DFB_LIBS) \
		-lavformat -lavcodec \
        `pkg-config --libs pangoft2`

# TODO: Remove -fpermissive
AM_CXXFLAGS = -fpermissive $(CXXFLAGS)

libplayerHEADERS = $(ALL_H)


