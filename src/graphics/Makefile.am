SUBDIRS = shaders

INCLUDES = -I.. @GL_CFLAGS@ @PYTHON_INCLUDES@ @GDK_PIXBUF_CFLAGS@

ALL_H = Bitmap.h Filter.h GLContext.h\
        Pixel32.h Pixel24.h Pixel16.h Pixel8.h Pixeldefs.h PixelFormat.h \
        Filtercolorize.h Filterfill.h Filterfillrect.h Filterflip.h FilterflipX.h \
        Filterfliprgb.h Filterflipuv.h Filtergrayscale.h Filter3x3.h \
        FilterId.h HistoryPreProcessor.h FilterConvol.h FilterHighpass.h \
        FilterFastBandpass.h Filterfliprgba.h FilterFastDownscale.h \
        FilterGauss.h FilterBandpass.h FilterBlur.h FilterMask.h \
        OGLHelper.h OGLShader.h GL/gl.h GL/glext.h GL/glu.h GL/glx.h \
        VertexArray.h OGLImagingContext.h GPUNullFilter.h GPUChromaKeyFilter.h \
        GPUBrightnessFilter.h GPUBlurFilter.h GPUShadowFilter.h GraphicsTest.h\
        GPUFilter.h GPUBandpassFilter.h GPUHueSatFilter.h GPUInvertFilter.h \
        FilterIntensity.h FilterNormalize.h FilterFloodfill.h FilterDilation.h \
        FilterErosion.h FilterGetAlpha.h FBO.h GLTexture.h PBO.h TextureMover.h\
        ContribDefs.h TwoPassScale.h FilterResizeBilinear.h FilterThreshold.h \
        FilterResizeGaussian.h FilterUnmultiplyAlpha.h ShaderRegistry.h \
        ImagingProjection.h BitmapManager.h BitmapManagerThread.h \
        BitmapManagerMsg.h GLBufferCache.h GLConfig.h BmpTextureMover.h \
        GPURGB2YUVFilter.h GLShaderParam.h StandardShader.h SubVertexArray.h \
        VertexData.h
ALL_CPP = Bitmap.cpp Filter.cpp Pixel32.cpp Filtergrayscale.cpp PixelFormat.cpp \
        Filtercolorize.cpp Filterflip.cpp FilterflipX.cpp Filterfliprgb.cpp \
        Filterflipuv.cpp Filter3x3.cpp HistoryPreProcessor.cpp FilterHighpass.cpp \
        FilterFastBandpass.cpp Filterfliprgba.cpp FilterFastDownscale.cpp \
        FilterGauss.cpp FilterBandpass.cpp FilterBlur.cpp FilterMask.cpp \
        OGLHelper.cpp OGLShader.cpp GPUNullFilter.cpp GPUChromaKeyFilter.cpp \
        GPUHueSatFilter.cpp GPUInvertFilter.cpp VertexArray.cpp OGLImagingContext.cpp \
        GPUBrightnessFilter.cpp GPUBlurFilter.cpp GPUShadowFilter.cpp GraphicsTest.cpp \
        GPUFilter.cpp GPUBandpassFilter.cpp FilterIntensity.cpp GLContext.cpp \
        FilterNormalize.cpp FilterDilation.cpp FilterErosion.cpp \
        FilterGetAlpha.cpp FBO.cpp GLTexture.cpp PBO.cpp TextureMover.cpp \
        FilterResizeBilinear.cpp FilterResizeGaussian.cpp FilterThreshold.cpp \
        FilterUnmultiplyAlpha.cpp ShaderRegistry.cpp \
        ImagingProjection.cpp BitmapManager.cpp BitmapManagerThread.cpp \
        BitmapManagerMsg.cpp GLBufferCache.cpp GLConfig.cpp BmpTextureMover.cpp \
        GPURGB2YUVFilter.cpp GLShaderParam.cpp StandardShader.cpp SubVertexArray.cpp \
        VertexData.cpp



if APPLE
    X_LIBS =
else
    X_LIBS = -lX11
endif

TESTS=testgraphics testgpu

EXTRA_DIST = $(wildcard baseline/*.png) 

noinst_LTLIBRARIES = libgraphics.la
libgraphics_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testgraphics testgpu benchmarkgraphics
testgraphics_SOURCES=testgraphics.cpp $(ALL_H)
testgraphics_LDADD = ./libgraphics.la ../base/libbase.la \
        ../base/triangulate/libtriangulate.la \
        @XML2_LIBS@ @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@ $(X_LIBS) @GDK_PIXBUF_LIBS@

benchmarkgraphics_SOURCES=benchmarkgraphics.cpp $(ALL_H)
benchmarkgraphics_LDADD = ./libgraphics.la ../base/libbase.la \
        ../base/triangulate/libtriangulate.la \
        @XML2_LIBS@ @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@ @GDK_PIXBUF_LIBS@

testgpu_SOURCES=testgpu.cpp $(ALL_H)
testgpu_LDADD = ./libgraphics.la ../base/libbase.la -ldl \
        ../base/triangulate/libtriangulate.la \
        @XML2_LIBS@ @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@ $(X_LIBS) \
        @GL_LIBS@ @GLU_LIBS@ @SDL_LIBS@ \
        @GDK_PIXBUF_LIBS@

