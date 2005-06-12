
INCLUDES = -I. \
        `xml2-config --cflags` \
        `paintlib-config --cflags` \
        `freetype-config --cflags` \
        `directfb-config --cflags` \
        `pkg-config --cflags pango` \
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
            
ALL_H = Player.h PlayerFactory.h IEventSource.h IEventSink.h \
        IDisplayEngine.h Rect.h Point.h Region.h Node.h \
        ISurface.h FramerateManager.h Container.h \
        MathHelper.h NodeFactory.h AVGNode.h DivNode.h AVGNodeFactory.h \
        ContainerFactory.h DivNodeFactory.h Image.h ImageFactory.h \
        Timeout.h Words.h WordsFactory.h \
        DebugEventSink.h EventDispatcher.h MouseEvent.h \
        Event.h KeyEvent.h EventFactory.h KeyEventFactory.h \
        MouseEventFactory.h VideoBase.h VideoBaseFactory.h \
        Video.h VideoFactory.h FFMpegDecoder.h IVideoDecoder.h \
        ExclFactory.h Excl.h PointFactory.h PanoImage.h PanoImageFactory.h\
        RasterNode.h RasterNodeFactory.h Camera.h CameraFactory.h\
        $(GL_INCLUDES) $(DFB_INCLUDES)

avgdir = $(libdir)/avg
avg_LTLIBRARIES = libplayer.la
libplayer_la_SOURCES = Player.cpp PlayerFactory.cpp \
        Region.cpp Node.cpp FramerateManager.cpp \
        Container.cpp  MathHelper.cpp \
        NodeFactory.cpp AVGNode.cpp DivNode.cpp AVGNodeFactory.cpp \
        ContainerFactory.cpp Image.cpp ImageFactory.cpp \
        Timeout.cpp Words.cpp WordsFactory.cpp \
        DebugEventSink.cpp EventDispatcher.cpp MouseEvent.cpp \
        Event.cpp KeyEvent.cpp KeyEventFactory.cpp\
        MouseEventFactory.cpp VideoBase.cpp VideoBaseFactory.cpp  \
        Video.cpp VideoFactory.cpp FFMpegDecoder.cpp \
        ExclFactory.cpp Excl.cpp PointFactory.cpp PanoImage.cpp \
        PanoImageFactory.cpp RasterNode.cpp RasterNodeFactory.cpp \
        Camera.cpp CameraFactory.cpp $(GL_SOURCES) $(DFB_SOURCES) \
        $(ALL_H)
libplayer_ladir=$(includedir)/avg/player
libplayer_la_HEADERS = $(ALL_H)

libplayer_la_LIBADD = ../base/libbase.la        

libplayer_la_LDFLAGS = `xml2-config --libs` `paintlib-config --libs` \
        $(DC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(SDL_LIBS) $(DFB_LIBS) -lavformat -lavcodec \
        `pkg-config --libs pangoft2`

# TODO: Remove -fpermissive
AM_CXXFLAGS = -DXP_UNIX -fpermissive $(CXXFLAGS)

libplayerHEADERS = $(ALL_H)


