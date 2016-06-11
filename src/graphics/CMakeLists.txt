if(${AVG_ENABLE_GLX_X11})
    set (GRAPHICS_SOURCES GLXContext.cpp PBO.cpp X11Display.cpp)
    set (GRAPHICS_LIBS ${GLX_X11_LDFLAGS})
    set (GRAPHICS_CFLAGS ${GLX_X11_CFLAGS})
endif()

if(${AVG_ENABLE_CGL_QUARTZ})
    set (GRAPHICS_SOURCES CGLContext.cpp PBO.cpp AppleDisplay.cpp)
    set (GRAPHICS_LIBS)
endif()

if(${AVG_ENABLE_EGL})
    set (GRAPHICS_SOURCES ${GRAPHICS_SOURCES} EGLContext.cpp)
endif()

if(${AVG_ENABLE_RPI})
    set (GRAPHICS_SOURCES ${GRAPHICS_SOURCES} BCMDisplay.cpp)
endif()

add_library(graphics
        ${GRAPHICS_SOURCES}
        Bitmap.cpp Filter.cpp Pixel32.cpp Filtergrayscale.cpp PixelFormat.cpp  
        Filtercolorize.cpp Filterflip.cpp FilterflipX.cpp Filterfliprgb.cpp 
        Filterflipuv.cpp Filter3x3.cpp FilterHighpass.cpp 
        Filterfliprgba.cpp FilterFastDownscale.cpp GLContextManager.cpp
        FilterGauss.cpp FilterBandpass.cpp FilterBlur.cpp FilterMask.cpp 
        OGLHelper.cpp OGLShader.cpp GPUNullFilter.cpp GPUChromaKeyFilter.cpp 
        Display.cpp GPUHueSatFilter.cpp GPUInvertFilter.cpp VertexArray.cpp
        GLContextAttribs.cpp GPUBrightnessFilter.cpp GPUBlurFilter.cpp
        GPUShadowFilter.cpp GraphicsTest.cpp 
        GPUFilter.cpp GPUBandpassFilter.cpp FilterIntensity.cpp GLContext.cpp 
        FilterNormalize.cpp FilterDilation.cpp FilterErosion.cpp 
        FilterGetAlpha.cpp FBO.cpp GLTexture.cpp TexInfo.cpp TextureMover.cpp 
        MCTexture.cpp FBOInfo.cpp MCFBO.cpp Color.cpp 
        FilterResizeBilinear.cpp FilterResizeGaussian.cpp FilterThreshold.cpp 
        FilterUnmultiplyAlpha.cpp ShaderRegistry.cpp
        ImagingProjection.cpp GLBufferCache.cpp GLConfig.cpp BmpTextureMover.cpp
        GPURGB2YUVFilter.cpp GLShaderParam.cpp StandardShader.cpp
        SubVertexArray.cpp VertexData.cpp BitmapLoader.cpp MCShaderParam.cpp
        CachedImage.cpp ImageCache.cpp WrapMode.cpp
)
target_link_libraries(graphics
    PUBLIC base ${GDK_PIXBUF_LDFLAGS} ${SDL2_LDFLAGS} ${GRAPHICS_LIBS})
target_compile_options(graphics
    PUBLIC ${GDK_PIXBUF_CFLAGS} ${SDL2_CFLAGS} ${GRAPHICS_CFLAGS})


link_libraries(graphics)
add_executable(testgraphics testgraphics.cpp)
add_executable(benchmarkgraphics benchmarkgraphics.cpp)
add_executable(testgpu testgpu.cpp)
add_test(NAME testgraphics
    COMMAND ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/testgraphics
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest)
add_test(NAME testgpu
    COMMAND ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/testgpu
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest)

include(testhelper)
copyTestToStaging(testgpu)
copyTestDataToStaging(testgpu baseline)
copyTestToStaging(testgraphics)
