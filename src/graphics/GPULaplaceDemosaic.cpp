#include "GPULaplaceDemosaic.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_STEP_1 "STEP_1"
#define SHADERID_STEP_2 "STEP_2"

using namespace std;

namespace avg {

GPULaplaceDemosaic::GPULaplaceDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 2)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
}

GPULaplaceDemosaic::~GPULaplaceDemosaic()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPULaplaceDemosaic::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step1_Shader = getShader(SHADERID_STEP_1);
    step1_Shader->activate();
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLShaderPtr step2_Shader = getShader(SHADERID_STEP_2);
    step2_Shader->activate();
    draw(getDestTex(0));

    glproc::UseProgramObject(0);
}

void GPULaplaceDemosaic::initShaders()
{
    string sProgramHead =
        "uniform sampler2D texture;\n"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

        /*
         * Linear Interpolation with Laplacian second-order correction terms
             * 
             * Step 1
         */
        "vec4 linlap_demosaic_step1()\n"
        "{\n"
        
        
        "   float dx = dFdx(gl_TexCoord[0].s);\n"
        "   float dy = dFdy(gl_TexCoord[0].t);\n"               
        /*
         *        |omm|
         *     |mm|om |pm|
         * |mmo|mo|tex|po|ppo|
         *     |mp|op |pp|
         *        |opp|
         */

             
            "   vec4 omm=texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy*2));\n"
            
            "   vec4 om =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy));\n" 

            "   vec4 mmo=texture2D(texture, gl_TexCoord[0].st+vec2(-dx*2,0));\n" 
        "   vec4 mo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,0));\n" 
        "   vec4 tex=texture2D(texture, gl_TexCoord[0].st);\n" 
        "   vec4 po =texture2D(texture, gl_TexCoord[0].st+vec2(dx,0));\n" 
            "   vec4 ppo=texture2D(texture, gl_TexCoord[0].st+vec2(dx*2,0));\n" 
            
        "   vec4 op =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy));\n"  
            
            "   vec4 opp=texture2D(texture, gl_TexCoord[0].st+vec2(0,dy*2));\n" 
        
            //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"


        "   vec4 ret = tex;\n"
        //Red
        "   if ( x_even && y_even ){ \n"
        "       float hor = abs(mo.g + po.g) + abs(2.0*tex.r - mmo.r - ppo.r);\n"
        "       float ver = abs(om.g + op.g) + abs(2.0*tex.r - omm.r - opp.r);\n"
        "       if (hor < ver){ \n"
        "           ret.g = 0.5*(mo.g + po.g) + 0.25*(2.0*tex.r - mmo.r - ppo.r);\n"
        "       } else {\n"
        "           if (ver < hor) { \n"
        "               ret.g = 0.5*(om.g + op.g) + 0.25*(2.0*tex.r - omm.r - opp.r);\n"
        "           } else {\n"
        "               ret.g = 0.25*(mo.g + po.g + om.g + op.g) + 0.125*(4.0*tex.r - mmo.r - ppo.r - omm.r - opp.r);\n"
        "           }\n"
        "       }\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       float hor = abs(mo.g + po.g) + abs(2.0*tex.b - mmo.b - ppo.b);\n"
        "       float ver = abs(om.g + op.g) + abs(2.0*tex.b - omm.b - opp.b);\n"
        "       if (hor < ver){ \n"
        "           ret.g = 0.5*(mo.g + po.g) + 0.25*(2.0*tex.b - mmo.b - ppo.b);\n"
        "       } else {\n"
        "           if (ver < hor) { \n"
        "               ret.g = 0.5*(om.g + op.g) + 0.25*(2.0*tex.b - omm.b - opp.b);\n"
        "           } else {\n"
        "               ret.g = 0.25*(mo.g + po.g + om.g + op.g) + 0.125*(4.0*tex.b - mmo.b - ppo.b - omm.b - opp.b);\n"
        "           }\n"
        "       }\n"
        "   }\n"
        "   return ret;\n"
        "}\n"

//----------------------------------------------------------------------------


        /*
         * Linear Interpolation with Laplacian second-order correction terms
             * 
             * Step 2
         */
        "vec4 linlap_demosaic_step2()\n"
        "{\n"
        
        
        "   float dx = dFdx(gl_TexCoord[0].s);\n"
        "   float dy = dFdy(gl_TexCoord[0].t);\n"               
         /*
          *     |mm|om |pm|
          *     |mo|tex|po|
          *     |mp|op |pp|
          */

            "   vec4 mm =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,-dy));\n" 
            "   vec4 om =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy));\n" 
        "   vec4 pm =texture2D(texture, gl_TexCoord[0].st+vec2(dx,-dy));\n"

        "   vec4 mo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,0));\n" 
        "   vec4 tex=texture2D(texture, gl_TexCoord[0].st);\n" 
        "   vec4 po =texture2D(texture, gl_TexCoord[0].st+vec2(dx,0));\n" 
            
            "   vec4 mp =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,dy));\n"
        "   vec4 op =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy));\n"
            "   vec4 pp =texture2D(texture, gl_TexCoord[0].st+vec2(dx,dy));\n"
        
        //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"

            "   vec4 ret = tex;\n"

        //Green with blue in same column
        "   if ( ( x_even && !y_even ) ){\n"

                //blue
            "            ret.b = 0.5*(mo.b + po.b) + 0.25*(2.0*tex.g - mo.g - po.g);\n"
                
                //red
        "       ret.r = 0.5*(om.r + op.r) + 0.25*(2.0*tex.g - om.g - op.g);\n"
        "   }\n"

            //Green with red in same column
            "   if ( ( !x_even && y_even ) ){\n"

                //blue
            "            ret.b = 0.5*(om.b + op.b) + 0.25*(2.0*tex.g - om.g - op.g);\n"
                
                //red
        "       ret.r = 0.5*(mo.r + po.r) + 0.25*(2.0*tex.g - mo.g - po.g);\n"
        "   }\n"

            //Red
        "   if ( x_even && y_even ){ \n"
            "            float d_N = abs(mm.b - pp.b) + abs(2.0*tex.g - mm.g - pp.g);\n"
            "            float d_P = abs(pm.b - mp.b) + abs(2.0*tex.g - pm.g - mp.g);\n"

        "       if (d_N < d_P){ \n"
        "           ret.b = 0.5*(mm.b + pp.b) + 0.25*(2.0*tex.g - mm.g - pp.g);\n"
        "       } else {\n"
        "           if (d_P < d_N) { \n"
        "               ret.b = 0.5*(pm.b + mp.b) + 0.25*(2.0*tex.g - pm.g - mp.g);\n"
        "           } else {\n"
        "               ret.b = 0.25*(mm.b + pp.b + pm.b + mp.b) + 0.125*(4.0*tex.g - mm.g - pp.g - pm.g - mp.g);\n"
        "           }\n"
        "       }\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "            float d_N = abs(mm.r - pp.r) + abs(2.0*tex.g - mm.g - pp.g);\n"
            "            float d_P = abs(pm.r - mp.r) + abs(2.0*tex.g - pm.g - mp.g);\n"

        "       if (d_N < d_P){ \n"
        "           ret.r = 0.5*(mm.r + pp.r) + 0.25*(2.0*tex.g - mm.g - pp.g);\n"
        "       } else {\n"
        "           if (d_P < d_N) { \n"
        "               ret.r = 0.5*(pm.r + mp.r) + 0.25*(2.0*tex.g - pm.g - mp.g);\n"
        "           } else {\n"
        "               ret.r = 0.25*(mm.r + pp.r + pm.r + mp.r) + 0.125*(4.0*tex.g - mm.g - pp.g - pm.g - mp.g);\n"
        "           }\n"
        "       }\n"
        "   }\n"

        "   return ret;\n"
        "}\n"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


        ;


    string step1_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        "      gl_FragColor.rgba = linlap_demosaic_step1().rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_1, step1_Program);

    string step2_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        "      gl_FragColor.rgba = linlap_demosaic_step2().rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_2, step2_Program);

}


}
