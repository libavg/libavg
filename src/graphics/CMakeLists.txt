add_library(graphics
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
    PUBLIC base ${GDK_PIXBUF_LDFLAGS} ${SDL2_LDFLAGS} )
target_compile_options(graphics
    PUBLIC ${GDK_PIXBUF_CFLAGS} ${SDL2_CFLAGS})

if(${AVG_ENABLE_GLX_X11})
    target_sources(graphics
        PRIVATE GLXContext.cpp PBO.cpp X11Display.cpp)
    target_link_libraries(graphics
        PUBLIC ${GLX_X11_LDFLAGS})
    target_compile_options(graphics
        PRIVATE ${GLX_X11_CFLAGS})
endif()
if(${AVG_ENABLE_CGL_QUARTZ})
    target_sources(graphics
        PRIVATE CGLContext.cpp PBO.cpp AppleDisplay.cpp)
    target_link_libraries(graphics
        PUBLIC ${PLATFORM_MACOSX_LDFLAGS})
    target_compile_options(graphics
        PUBLIC ${PLATFORM_MACOSX_CFLAGS})
endif()
if(${AVG_ENABLE_EGL})
    target_sources(graphics
        PRIVATE EGLContext.cpp)
endif()
if(${AVG_ENABLE_BCM})
    target_sources(graphics
        PRIVATE BCMDisplay.cpp)
endif()

link_libraries(graphics)
add_executable(testgraphics testgraphics.cpp)
add_executable(benchmarkgraphics benchmarkgraphics.cpp)
add_executable(testgpu testgpu.cpp)

