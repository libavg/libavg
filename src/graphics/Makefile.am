INCLUDES = -I.. $(MAGICK_CFLAGS) @GL_CFLAGS@

ALL_H = Bitmap.h Filter.h\
        Pixel32.h Pixel24.h Pixel16.h Pixel8.h Pixeldefs.h \
        Filtercolorize.h Filterfill.h Filterfillrect.h Filterflip.h FilterflipX.h \
        Filterfliprgb.h Filterflipuv.h Filtergrayscale.h Filter3x3.h \
        FilterId.h HistoryPreProcessor.h FilterConvol.h FilterHighpass.h \
        FilterFastBandpass.h Filterfliprgba.h FilterFastDownscale.h \
        FilterGauss.h FilterBandpass.h FilterBlur.h FilterMask.h \
        OGLHelper.h OGLShader.h GL/gl.h GL/glext.h GL/glu.h GL/glx.h \
        PBOImage.h VertexArray.h OGLImagingContext.h \
        GPUBrightnessFilter.h GPUBlurFilter.h GraphicsTest.h\
        GPUFilter.h GPUBandpassFilter.h FilterIntensity.h \
        FilterNormalize.h FilterFloodfill.h FilterDilation.h FilterErosion.h \
        FilterGetAlpha.h FBO.h \
        ContribDefs.h TwoPassScale.h FilterResizeBilinear.h \
        FilterResizeGaussian.h 
ALL_CPP = Bitmap.cpp Filter.cpp Pixel32.cpp Filtergrayscale.cpp \
        Filtercolorize.cpp Filterflip.cpp FilterflipX.cpp Filterfliprgb.cpp \
        Filterflipuv.cpp Filter3x3.cpp HistoryPreProcessor.cpp FilterHighpass.cpp \
        FilterFastBandpass.cpp Filterfliprgba.cpp FilterFastDownscale.cpp \
        FilterGauss.cpp FilterBandpass.cpp FilterBlur.cpp FilterMask.cpp \
        OGLHelper.cpp OGLShader.cpp \
        PBOImage.cpp VertexArray.cpp OGLImagingContext.cpp \
        GPUBrightnessFilter.cpp GPUBlurFilter.cpp GraphicsTest.cpp \
        GPUFilter.cpp GPUBandpassFilter.cpp FilterIntensity.cpp \
        FilterNormalize.cpp FilterDilation.cpp FilterErosion.cpp \
        FilterGetAlpha.cpp FBO.cpp \
        FilterResizeBilinear.cpp FilterResizeGaussian.cpp

TESTS=testgraphics testgpu

EXTRA_DIST = $(wildcard baseline/*.png) $(wildcard testfiles/*.png) 

noinst_LTLIBRARIES = libgraphics.la
libgraphics_la_SOURCES = $(ALL_CPP) $(ALL_H)
noinst_PROGRAMS=testgraphics testgpu benchmarkgraphics
testgraphics_SOURCES=testgraphics.cpp $(ALL_H)
testgraphics_LDADD = ./libgraphics.la ../base/libbase.la
testgraphics_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(MAGICK_LIBS)

benchmarkgraphics_SOURCES=benchmarkgraphics.cpp $(ALL_H)
benchmarkgraphics_LDADD = ./libgraphics.la ../base/libbase.la
benchmarkgraphics_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(MAGICK_LIBS)

testgpu_SOURCES=testgpu.cpp $(ALL_H)
testgpu_LDADD = ./libgraphics.la ../base/libbase.la
testgpu_LDFLAGS = @XML2_LIBS@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ $(MAGICK_LIBS) @GL_LIBS@ \
        @GLU_LIBS@ @SDL_LIBS@
