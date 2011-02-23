

#include "GPUDemosaic.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_HORIZ "HORIZBLUR"
#define SHADERID_VERT "VERTBLUR"

using namespace std;

namespace avg {

GPUDemosaic::GPUDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        double stdDev, bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 2),
      m_StdDev(stdDev)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev);
}

GPUDemosaic::~GPUDemosaic()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUDemosaic::setParam(double stdDev)
{
    m_StdDev = stdDev;
    m_pGaussCurveTex = calcBlurKernelTex(m_StdDev);
}

void GPUDemosaic::applyOnGPU(GLTexturePtr pSrcTex)
{
    int kernelWidth = m_pGaussCurveTex->getSize().x;
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLShaderPtr pHShader = getShader(SHADERID_HORIZ);
    pHShader->activate();
    pHShader->setUniformFloatParam("width", float(kernelWidth));
    pHShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pHShader->setUniformIntParam("texture", 0);
    pHShader->setUniformIntParam("kernelTex", 1);
    m_pGaussCurveTex->activate(GL_TEXTURE1);
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr pVShader = getShader(SHADERID_VERT);
    pVShader->activate();
    pVShader->setUniformFloatParam("width", float(kernelWidth));
    pVShader->setUniformIntParam("radius", (kernelWidth-1)/2);
    pVShader->setUniformIntParam("texture", 0);
    pVShader->setUniformIntParam("kernelTex", 1);
    draw(getDestTex(1));
    glproc::UseProgramObject(0);
}

void GPUDemosaic::initShaders()
{
    string sProgramHead =
        "uniform sampler2D texture;\n"
        "uniform float width;\n"
        "uniform int radius;\n"
        "uniform sampler2D kernelTex;\n"
        ;


         string sHorizProgram = sProgramHead +
        "void main(void)\n"
        "{\n"
//        "    vec4 sum = vec4(0,0,0,0);\n"
        "    float dx = dFdx(gl_TexCoord[0].x);\n"
          "    float dy = dFdy(gl_TexCoord[0].y);\n"               
               /*
                * |mm|om |pm|
                * |mo|tex|po|
                * |mp|op |pp|
                */
               "vec4 mm =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,-dy));\n" 
               "vec4 om =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy));\n" 
               "vec4 pm =texture2D(texture, gl_TexCoord[0].st+vec2(dx,-dy));\n" 

               "vec4 mo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,0));\n" 
               "vec4 tex =texture2D(texture, gl_TexCoord[0].st);\n" 
               "vec4 po =texture2D(texture, gl_TexCoord[0].st+vec2(dx,0));\n" 

               "vec4 mp =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,dy));\n"
               "vec4 op =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy));\n" 
               "vec4 pp =texture2D(texture, gl_TexCoord[0].st+vec2(dx,dy));\n" 

               //evaluate if the coordinates of the current pixel are even
               "bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
               "bool y_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
               //Red
               "if ( x_even && y_even ){ \n"
               "     gl_FragColor.r = tex.r;\n"
               "     gl_FragColor.g = (om.g + mo.g + op.g + po.g)/4.0;\n"
               "     gl_FragColor.b = (mm.b + pm.b + mp.b + pp.b)/4.0;\n"
               "}\n"
               
               //Blue
               "if ( !x_even && !y_even ){\n"
               "     gl_FragColor.r = (mm.r + pm.r + mp.r + pp.r)/4.0;\n"
               "     gl_FragColor.g = (om.g + mo.g + op.g + po.g)/4.0;\n"
               "     gl_FragColor.b = tex.b;\n"
               "}\n"

               //Green
               "if ( ( !x_even && y_even ) ){\n"
               "     gl_FragColor.r = (mo.r + po.r)/2.0;\n"
               "     gl_FragColor.g = tex.g;\n"
               "     gl_FragColor.b = (om.b + op.b)/2.0;\n"
               "}\n"
               "if ( x_even && !y_even ){\n"
               "     gl_FragColor.r = (om.r + op.r)/2.0;\n"
               "     gl_FragColor.g = tex.g;\n"
               "     gl_FragColor.b = (mo.b + po.b)/2.0;\n"
               "}\n"
          "      gl_FragColor.a = tex.a;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_HORIZ, sHorizProgram);

    string sVertProgram = sProgramHead +
        "void main(void)\n"
        "{\n"
        //"    vec4 sum = vec4(0,0,0,0);\n"
        //"    float dy = dFdy(gl_TexCoord[0].y);\n"
        //"    for (int i=-radius; i<=radius; ++i) {\n"
        //"        vec4 tex = texture2D(texture, gl_TexCoord[0].st+vec2(0,float(i)*dy));\n"
        //"        float coeff = \n"
        //"                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        //"        sum += tex*coeff;\n"
        //"    }\n"
        //"    gl_FragColor = sum;\n"
          "      vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n" 
          "      gl_FragColor= tex;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_VERT, sVertProgram);

     /*
    string sHorizProgram = sProgramHead +
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    float dx = dFdx(gl_TexCoord[0].x);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2D(texture, gl_TexCoord[0].st+vec2(float(i)*dx,0));\n"
        "        float coeff = \n"
        "                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        "        sum += tex*coeff;\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_HORIZ, sHorizProgram);

    string sVertProgram = sProgramHead +
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0,0,0,0);\n"
        "    float dy = dFdy(gl_TexCoord[0].y);\n"
        "    for (int i=-radius; i<=radius; ++i) {\n"
        "        vec4 tex = texture2D(texture, gl_TexCoord[0].st+vec2(0,float(i)*dy));\n"
        "        float coeff = \n"
        "                texture2D(kernelTex, vec2((float(i+radius)+0.5)/width,0)).r;\n"
        "        sum += tex*coeff;\n"
        "    }\n"
        "    gl_FragColor = sum;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_VERT, sVertProgram);
     */
}


}
