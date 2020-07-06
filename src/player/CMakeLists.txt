add_library(player
    AreaNode.cpp RasterNode.cpp DivNode.cpp VideoNode.cpp ExportedObject.cpp
    Player.cpp PluginManager.cpp TypeRegistry.cpp ArgBase.cpp ArgList.cpp
    DisplayEngine.cpp Canvas.cpp CanvasNode.cpp OffscreenCanvasNode.cpp
    MainCanvas.cpp Node.cpp MultitouchInputDevice.cpp WrapPython.cpp
    WordsNode.cpp CameraNode.cpp TypeDefinition.cpp TextEngine.cpp
    Timeout.cpp Event.cpp DisplayParams.cpp WindowParams.cpp CursorState.cpp
    GPUImage.cpp ImageNode.cpp EventDispatcher.cpp KeyEvent.cpp
    CursorEvent.cpp MouseEvent.cpp TouchEvent.cpp AVGNode.cpp TestHelper.cpp
    SoundNode.cpp FontStyle.cpp Window.cpp SDLWindow.cpp MouseWheelEvent.cpp
    TangibleEvent.cpp InputDevice.cpp SecondaryWindow.cpp
    VectorNode.cpp  FilledVectorNode.cpp LineNode.cpp PolyLineNode.cpp
    RectNode.cpp CurveNode.cpp PolygonNode.cpp CircleNode.cpp Shape.cpp MeshNode.cpp
    Contact.cpp TouchStatus.cpp OffscreenCanvas.cpp FXNode.cpp TUIOInputDevice.cpp
    NullFXNode.cpp BlurFXNode.cpp ShadowFXNode.cpp ChromaKeyFXNode.cpp
    InvertFXNode.cpp HueSatFXNode.cpp VideoWriter.cpp VideoWriterThread.cpp
    SVG.cpp SVGElement.cpp Publisher.cpp SubscriberInfo.cpp PublisherDefinition.cpp
    PublisherDefinitionRegistry.cpp MessageID.cpp VersionInfo.cpp
    PythonLogSink.cpp BitmapManager.cpp BitmapManagerThread.cpp
    BitmapManagerMsg.cpp SDLTouchInputDevice.cpp NodeChain.cpp
    OGLSurface.cpp)
add_dependencies(player version)
target_link_libraries(player
    PUBLIC video imaging graphics oscpack
        ${PYTHON_LIBRARIES} ${PANGOFT2_LDFLAGS} ${RSVG_LDFLAGS} ${FONTCONFIG_STATIC_LDFLAGS})
target_include_directories(player
    PUBLIC SYSTEM ${PYTHON_INCLUDE_DIRS})
target_compile_options(player
    PUBLIC ${PANGOFT2_CFLAGS} ${RSVG_CFLAGS})



link_libraries(player)
add_executable(testplayer testplayer.cpp)
if(${PLATFORM_LINUX})
    # add -lpthread (done by boost-thread on most systems, but missing on some)
    target_link_libraries(testplayer PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()
add_test(NAME testplayer
    COMMAND ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/testplayer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest)

include(testhelper)
copyTestToStaging(testplayer)
