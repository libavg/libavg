AM_CPPFLAGS = -I.. @XML2_CFLAGS@ @PYTHON_CPPFLAGS@ @GL_CFLAGS@ \
	@PANGOFT2_CFLAGS@ @LIBRSVG_CFLAGS@ @FONTCONFIG_CFLAGS@ @FREETYPE_CFLAGS@

if APPLE
    APPLE_LINKFLAGS = -F/System/Library/PrivateFrameworks \
            -framework MultitouchSupport -framework Cocoa -framework CoreAudio \
            -framework AudioUnit -framework ForceFeedback
    XGL_LIBS =
else
    APPLE_LINKFLAGS =
if ENABLE_RPI
    XGL_LIBS = -lXxf86vm -lX11 -lbcm_host -lpthread -lGLESv2 -lEGL
    XGL_LINKFLAGS =
else
if ENABLE_EGL
    XGL_LIBS = -lGLESv2 -lEGL
    XGL_LINKFLAGS = 
else
    XGL_LIBS = -lXxf86vm -lX11
    XGL_LINKFLAGS =
endif
endif
endif

GL_SOURCES = OGLSurface.cpp
GL_INCLUDES = OGLSurface.h

ALL_GL_LIBS = @GL_LIBS@ @SDL_LIBS@ $(XGL_LIBS)

ALL_H = Player.h PluginManager.h InputDevice.h ExportedObject.h \
        DisplayEngine.h TypeRegistry.h Arg.h ArgBase.h ArgList.h \
        Node.h AreaNode.h DisplayParams.h WindowParams.h TypeDefinition.h TextEngine.h \
        AVGNode.h DivNode.h CursorState.h Canvas.h MainCanvas.h \
        GPUImage.h ImageNode.h Timeout.h WordsNode.h WrapPython.h OffscreenCanvas.h \
        EventDispatcher.h CursorEvent.h MouseEvent.h MouseWheelEvent.h \
        Event.h KeyEvent.h TestHelper.h CanvasNode.h \
        OffscreenCanvasNode.h MultitouchInputDevice.h \
        RasterNode.h CameraNode.h SecondaryWindow.h \
        TouchEvent.h Contact.h TouchStatus.h BoostPython.h \
        FontStyle.h Window.h SDLWindow.h TangibleEvent.h \
        VectorNode.h FilledVectorNode.h LineNode.h PolyLineNode.h RectNode.h \
        CurveNode.h PolygonNode.h CircleNode.h Shape.h MeshNode.h FXNode.h \
        NullFXNode.h BlurFXNode.h ShadowFXNode.h ChromaKeyFXNode.h HueSatFXNode.h \
        InvertFXNode.h TUIOInputDevice.h \
        SVG.h SVGElement.h Publisher.h SubscriberInfo.h PublisherDefinition.h \
        PublisherDefinitionRegistry.h MessageID.h VersionInfo.h \
        PythonLogSink.h BitmapManager.h BitmapManagerThread.h IBitmapLoadedListener.h \
        BitmapManagerMsg.h SDLTouchInputDevice.h \
        $(GL_INCLUDES)

TESTS = testplayer

noinst_LTLIBRARIES = libplayer.la
noinst_PROGRAMS = testplayer
testplayer_SOURCES = testplayer.cpp
testplayer_LDADD = libplayer.la ../audio/libaudio.la \
        ../imaging/libimaging.la ../graphics/libgraphics.la ../base/libbase.la \
        ../oscpack/liboscpack.la ../tess/libtess.la \
        @XML2_LIBS@ @BOOST_THREAD_LIBS@ @PTHREAD_CFLAGS@ @PTHREAD_LIBS@ @PANGOFT2_LIBS@ \
        @LIBRSVG_LIBS@ \
        @DC1394_2_LIBS@ $(ALL_GL_LIBS) \
        $(BOOST_PYTHON_LIBS) $(PYTHON_LDFLAGS) -lpython2.7 @GDK_PIXBUF_LIBS@ \
        @FONTCONFIG_LIBS@

testplayer_LDFLAGS = $(APPLE_LINKFLAGS) -module -XCClinker $(XGL_LINKFLAGS)

libplayer_la_LIBADD = $(BOOST_PYTHON_LIBS) $(PYTHON_LDFLAGS)
libplayer_la_SOURCES = $(GL_SOURCES) \
        Arg.cpp AreaNode.cpp RasterNode.cpp DivNode.cpp ExportedObject.cpp \
        Player.cpp PluginManager.cpp TypeRegistry.cpp ArgBase.cpp ArgList.cpp \
        DisplayEngine.cpp Canvas.cpp CanvasNode.cpp OffscreenCanvasNode.cpp \
        MainCanvas.cpp Node.cpp MultitouchInputDevice.cpp WrapPython.cpp \
        WordsNode.cpp CameraNode.cpp TypeDefinition.cpp TextEngine.cpp \
        Timeout.cpp Event.cpp DisplayParams.cpp WindowParams.cpp CursorState.cpp \
        GPUImage.cpp ImageNode.cpp EventDispatcher.cpp KeyEvent.cpp \
        CursorEvent.cpp MouseEvent.cpp TouchEvent.cpp AVGNode.cpp TestHelper.cpp \
        FontStyle.cpp Window.cpp SDLWindow.cpp MouseWheelEvent.cpp \
        TangibleEvent.cpp InputDevice.cpp SecondaryWindow.cpp \
        VectorNode.cpp  FilledVectorNode.cpp LineNode.cpp PolyLineNode.cpp \
        RectNode.cpp CurveNode.cpp PolygonNode.cpp CircleNode.cpp Shape.cpp MeshNode.cpp \
        Contact.cpp TouchStatus.cpp OffscreenCanvas.cpp FXNode.cpp TUIOInputDevice.cpp \
        NullFXNode.cpp BlurFXNode.cpp ShadowFXNode.cpp ChromaKeyFXNode.cpp \
        InvertFXNode.cpp HueSatFXNode.cpp \
        SVG.cpp SVGElement.cpp Publisher.cpp SubscriberInfo.cpp PublisherDefinition.cpp \
        PublisherDefinitionRegistry.cpp MessageID.cpp VersionInfo.cpp \
        PythonLogSink.cpp BitmapManager.cpp BitmapManagerThread.cpp \
        BitmapManagerMsg.cpp SDLTouchInputDevice.cpp \
        $(ALL_H)
libplayer_a_CXXFLAGS = -DPREFIXDIR=\"$(prefix)\"
