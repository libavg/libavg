#include "GPUBilinDemosaic.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

#define SHADERID_STEP_1 "STEP_1"

using namespace std;

namespace avg {

GPUBilinDemosaic::GPUBilinDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 1)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
}

GPUBilinDemosaic::~GPUBilinDemosaic()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}
      
void GPUBilinDemosaic::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step1_Shader = getShader(SHADERID_STEP_1);
    step1_Shader->activate();
    step1_Shader->setUniformIntParam("texture", 0);
    draw(pSrcTex);

    glproc::UseProgramObject(0);
}

void GPUBilinDemosaic::initShaders()
{
    string sProgramHead =
        "uniform sampler2D texture;\n"

        /*
         * Bilinear color interpolation scheme
         */
        "vec4 bilin_demosaic()\n"
        "{\n"
        "   vec4 rgb = vec4(0.0);\n"
        
        "   float dx = dFdx(gl_TexCoord[0].s);\n"
        "   float dy = dFdy(gl_TexCoord[0].t);\n"               
         /*
          * |mm|om |pm|
          * |mo|tex|po|
          * |mp|op |pp|
          */
        "   vec4 mm =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,-dy));\n" 
        "   vec4 om =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy));\n" 
        "   vec4 pm =texture2D(texture, gl_TexCoord[0].st+vec2(dx,-dy));\n" 
          
        "   vec4 mo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,0));\n" 
        "   vec4 tex =texture2D(texture, gl_TexCoord[0].st);\n" 
        "   vec4 po =texture2D(texture, gl_TexCoord[0].st+vec2(dx,0));\n" 
         
        "   vec4 mp =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,dy));\n"
        "   vec4 op =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy));\n" 
        "   vec4 pp =texture2D(texture, gl_TexCoord[0].st+vec2(dx,dy));\n" 
        
        //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"
        //Red
        "   if ( x_even && y_even ){ \n"
        "       rgb.r = tex.r;\n"
        "       rgb.g = (om.g + mo.g + op.g + po.g)/4.0;\n"
        "       rgb.b = (mm.b + pm.b + mp.b + pp.b)/4.0;\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       rgb.r = (mm.r + pm.r + mp.r + pp.r)/4.0;\n"
        "       rgb.g = (om.g + mo.g + op.g + po.g)/4.0;\n"
        "       rgb.b = tex.b;\n"
        "   }\n"
        
        //Green
        "   if ( ( !x_even && y_even ) ){\n"
        "       rgb.r = (mo.r + po.r)/2.0;\n"
        "       rgb.g = tex.g;\n"
        "       rgb.b = (om.b + op.b)/2.0;\n"
        "   }\n"
        "   if ( x_even && !y_even ){\n"
        "       rgb.r = (om.r + op.r)/2.0;\n"
        "       rgb.g = tex.g;\n"
        "       rgb.b = (mo.b + po.b)/2.0;\n"
        "   }\n"
        "   rgb.a = tex.a;\n"
        "   return rgb;\n"
        "}\n"

        ;


    string step1_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        "      gl_FragColor.rgba = bilin_demosaic();\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_1, step1_Program);
}


}
