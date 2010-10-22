INCLUDES = -I.. @XML2_CFLAGS@ @FREETYPE_CFLAGS@ \
        @PANGOFT2_CFLAGS@ @PYTHON_INCLUDES@ @GL_CFLAGS@ @FFMPEG_CFLAGS@ \
        @DC1394_2_CFLAGS@ \
        $(MAGICK_CFLAGS)
        
if APPLE
    APPLE_SOURCES = SDLMain.m AppleTrackpadEventSource.cpp
    XGL_LIBS =
    APPLE_LINKFLAGS=-read_only_relocs suppress -F/System/Library/PrivateFrameworks -framework MultitouchSupport
else
    APPLE_SOURCES =
    XGL_LIBS = -lXxf86vm
    APPLE_LINKFLAGS=
endif

GL_SOURCES = OGLSurface.cpp SDLDisplayEngine.cpp PBOTexture.cpp GLConfig.cpp
GL_INCLUDES = OGLSurface.h SDLDisplayEngine.h PBOTexture.h GLConfig.h

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

ALL_H = Player.h PluginManager.h IEventSource.h IEventSink.h VideoNode.h \
        DisplayEngine.h NodeRegistry.h Arg.h ArgBase.h ArgList.h \
        Node.h VisibleNode.h AreaNode.h DisplayParams.h NodeDefinition.h TextEngine.h \
        AVGNode.h DivNode.h CursorState.h MaterialInfo.h Canvas.h MainCanvas.h \
        Image.h ImageNode.h Timeout.h WordsNode.h WrapPython.h OffscreenCanvas.h \
        EventDispatcher.h CursorEvent.h MouseEvent.h \
        Event.h KeyEvent.h PanoImageNode.h TestHelper.h CanvasNode.h \
        OffscreenCanvasNode.h \
        RasterNode.h CameraNode.h TrackerEventSource.h TrackerCalibrator.h \
        TouchEvent.h Touch.h EventStream.h BoostPython.h SoundNode.h MouseState.h \
        VectorNode.h FilledVectorNode.h LineNode.h PolyLineNode.h RectNode.h \
        CurveNode.h PolygonNode.h CircleNode.h Shape.h MeshNode.h FXNode.h \
        NullFXNode.h BlurFXNode.h ShadowFXNode.h ChromaKeyFXNode.h \
        $(GL_INCLUDES)

TESTS=testcalibrator testplayer

EXTRA_DIST = SDLMain.h

noinst_LTLIBRARIES = libplayer.la
noinst_PROGRAMS = checktracker testcalibrator testplayer
checktracker_SOURCES=checktracker.cpp
checktracker_LDADD = libplayer.la ../video/libvideo.la ../audio/libaudio.la \
        ../imaging/libimaging.la ../graphics/libgraphics.la ../base/libbase.la \
        ../lmfit/liblmfit.la
checktracker_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ \
        @DC1394_2_LIBS@ @GLU_LIBS@ $(ALL_GL_LIBS) $(MAGICK_LIBS) \
        $(BOOST_PYTHON_LIBS) $(PYTHON_LIBS)

testplayer_SOURCES=testplayer.cpp
testplayer_LDADD = libplayer.la ../video/libvideo.la ../audio/libaudio.la \
        ../imaging/libimaging.la ../graphics/libgraphics.la ../base/libbase.la \
        ../lmfit/liblmfit.la
testplayer_LDFLAGS = $(APPLE_LINKFLAGS) -module -XCClinker  \
        @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ \
        @DC1394_2_LIBS@ @GLU_LIBS@ $(ALL_GL_LIBS) @LIBFFMPEG@ \
        @PANGOFT2_LIBS@ $(BOOST_PYTHON_LIBS) $(PYTHON_LIBS) $(MAGICK_LIBS) 

testcalibrator_SOURCES=testcalibrator.cpp
testcalibrator_LDADD = libplayer.la ../video/libvideo.la ../audio/libaudio.la \
        ../imaging/libimaging.la ../graphics/libgraphics.la ../base/libbase.la \
        ../lmfit/liblmfit.la
testcalibrator_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(MAGICK_LIBS)

libplayer_la_LDFLAGS = $(BOOST_PYTHON_LIBS) $(PYTHON_LIBS)
libplayer_la_SOURCES = $(GL_SOURCES) \
        Arg.cpp VisibleNode.cpp AreaNode.cpp RasterNode.cpp DivNode.cpp VideoNode.cpp \
        Player.cpp PluginManager.cpp NodeRegistry.cpp ArgBase.cpp ArgList.cpp \
        DisplayEngine.cpp Canvas.cpp CanvasNode.cpp OffscreenCanvasNode.cpp \
        MainCanvas.cpp Node.cpp \
        PanoImageNode.cpp WordsNode.cpp CameraNode.cpp NodeDefinition.cpp TextEngine.cpp \
        Timeout.cpp Event.cpp DisplayParams.cpp CursorState.cpp MaterialInfo.cpp \
        Image.cpp ImageNode.cpp EventDispatcher.cpp KeyEvent.cpp CursorEvent.cpp \
        MouseEvent.cpp TouchEvent.cpp AVGNode.cpp TestHelper.cpp MouseState.cpp \
        TrackerEventSource.cpp EventStream.cpp TrackerCalibrator.cpp SoundNode.cpp \
        VectorNode.cpp  FilledVectorNode.cpp LineNode.cpp PolyLineNode.cpp \
        RectNode.cpp CurveNode.cpp PolygonNode.cpp CircleNode.cpp Shape.cpp MeshNode.cpp \
        Touch.cpp OffscreenCanvas.cpp FXNode.cpp \
        NullFXNode.cpp BlurFXNode.cpp ShadowFXNode.cpp ChromaKeyFXNode.cpp \
        $(APPLE_SOURCES) $(ALL_H)
libplayer_a_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
