#include "GPUBayerize.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_STEP_1 "STEP_1"

using namespace std;

namespace avg {

GPUBayerize::GPUBayerize(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 1)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
}

GPUBayerize::~GPUBayerize()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBayerize::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step1_Shader = getShader(SHADERID_STEP_1);
    step1_Shader->activate();
    step1_Shader->setUniformIntParam("texture", 0);
    draw(pSrcTex);

    glproc::UseProgramObject(0);
}

void GPUBayerize::initShaders()
{
    string sProgramHead =
        "uniform sampler2D texture;\n"

        /* 
         * Delete colors s.t. the image looks like a bayer-pattern
         */
        "vec4 bayerize()\n"
        "{\n"
        "   vec4 rgb = vec4(0.0);\n"
        
        "   float dx = dFdx(gl_TexCoord[0].s);\n"
        "   float dy = dFdy(gl_TexCoord[0].t);\n"               

        "   vec4 tex =texture2D(texture, gl_TexCoord[0].st);\n" 
        

        //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"
        //Red
        "   if ( x_even && y_even ){ \n"
        "       rgb.r = tex.r;\n"
        "       rgb.gb = vec2(0.0);\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       rgb.b = tex.b;\n"
        "       rgb.rg = vec2(0.0);\n"
        "   }\n"
       
        //Green
        "   if ( ( !x_even && y_even ) ){\n"
        "       rgb.g = tex.g;\n"
        "       rgb.rb = vec2(0.0);\n"
        "   }\n"
        "   if ( x_even && !y_even ){\n"
        "       rgb.g = tex.g;\n"
       "        rgb.rb = vec2(0.0);\n"
        "   }\n"
        "   rgb.a = tex.a;\n"
        "   return rgb;\n"
        "}\n"
        "\n"

        ;


    string step1_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        "      gl_FragColor.rgba = bayerize();\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_1, step1_Program);
}


}
