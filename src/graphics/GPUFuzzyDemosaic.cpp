#include "GPUFuzzyDemosaic.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID_STEP_1 "STEP_1"
#define SHADERID_STEP_2 "STEP_2"
#define SHADERID_STEP_3 "STEP_3"

using namespace std;

namespace avg {

GPUFuzzyDemosaic::GPUFuzzyDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 2)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    
    initShaders();
}

GPUFuzzyDemosaic::~GPUFuzzyDemosaic()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUFuzzyDemosaic::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step1_Shader = getShader(SHADERID_STEP_1);
    step1_Shader->activate();
    step1_Shader->setUniformIntParam("texture", 0);
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLShaderPtr step2_Shader = getShader(SHADERID_STEP_2);
    step2_Shader->activate();
    step2_Shader->setUniformIntParam("texture", 0);
    draw(getDestTex(1));

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step3_Shader = getShader(SHADERID_STEP_3);
    step3_Shader->activate();
    step3_Shader->setUniformIntParam("texture", 0);
    draw(getDestTex(0));

    glproc::UseProgramObject(0);
}

void GPUFuzzyDemosaic::initShaders()
{
    string sProgramHead =
        "uniform sampler2D texture;\n"
      
//----------------------------------------------------------------------------

        /*
         * Adaptive fuzzy color interpolation step 1
         */
        "vec4 fuzzy_demosaic_step1()\n"
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

        "   vec4 omm =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy*2));\n"
        "   vec4 om =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy));\n"

        "   vec4 mmo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx*2,0));\n" 
        "   vec4 mo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,0));\n" 
        "   vec4 tex =texture2D(texture, gl_TexCoord[0].st);\n" 
        "   vec4 po =texture2D(texture, gl_TexCoord[0].st+vec2(dx,0));\n" 
        "   vec4 ppo =texture2D(texture, gl_TexCoord[0].st+vec2(dx*2,0));\n" 
         
        "   vec4 op =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy));\n" 
        "   vec4 opp =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy*2));\n" 

        //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"

            "   vec4 ret = tex;\n"

        //Red
        "   if ( x_even && y_even ){ \n"
        "       float c_hor = 0.5*(-mmo.r + 2.0*mo.g - 2.0*po.g + ppo.r);\n"
        "       float c_ver = 0.5*(-omm.r + 2.0*om.g - 2.0*op.g + opp.r);\n"
        "       float i_ver = 0.5*(om.g + op.g) + 0.125*(-omm.r + 2.0*tex.r - opp.r);\n"
        "       float i_hor = 0.5*(mo.g + po.g) + 0.125*(-mmo.r + 2.0*tex.r - ppo.r);\n"
        "       if (abs(c_hor)<abs(c_ver)){ \n"
        "           ret.g = 0.8333*i_hor+0.1667*i_ver;\n"
        "       } else {\n"
        "           if (abs(c_ver)<abs(c_hor)) { \n"
        "               ret.g = 0.8333*i_ver+0.1667*i_hor;\n"
        "           } else {\n"
        "               ret.g = 0.5*i_ver+0.5*i_hor;\n"
        "           }\n"
        "       }\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       float c_hor = 0.5*(-mmo.b + 2.0*mo.g - 2.0*po.g + ppo.b);\n"
        "       float c_ver = 0.5*(-omm.b + 2.0*om.g - 2.0*op.g + opp.b);\n"
        "       float i_ver = 0.5*(om.g + op.g) + 0.125*(-omm.b + 2.0*tex.b - opp.b);\n"
        "       float i_hor = 0.5*(mo.g + po.g) + 0.125*(-mmo.b + 2.0*tex.b - ppo.b);\n"
        "       if (abs(c_hor)<abs(c_ver)){ \n"
        "           ret.g = 0.8333*i_hor+0.1667*i_ver;\n"
        "       } else {\n"
        "           if (abs(c_ver)<abs(c_hor)) { \n"
        "               ret.g = 0.8333*i_ver+0.1667*i_hor;\n"
        "           } else {\n"
        "               ret.g = 0.5*i_ver+0.5*i_hor;\n"
        "           }\n"
        "       }\n"
        "   }\n"
        "   return ret;\n"
        "}\n"



//----------------------------------------------------------------------------

        /*
         * Adaptive fuzzy color interpolation step 2
         */
        "vec4 fuzzy_demosaic_step2()\n"
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
        "   vec4 mm =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,-dy));\n" 
        "   vec4 pm =texture2D(texture, gl_TexCoord[0].st+vec2(dx,-dy));\n" 

        "   vec4 tex =texture2D(texture, gl_TexCoord[0].st);\n" 
 
        "   vec4 mp =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,dy));\n"
        "   vec4 pp =texture2D(texture, gl_TexCoord[0].st+vec2(dx,dy));\n" 
        
        //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"

            "   vec4 ret = tex;\n"
        
        //Red
        "   if ( x_even && y_even ){ \n"
        "       float h_nw = mm.b - mm.g;\n"
        "       float h_sw = mp.b - mp.g;\n"
        "       float h_ne = pm.b - pm.g;\n"
        "       float h_se = pp.b - pp.g;\n"
        "       float h_md = h_nw - h_se;\n"
        "       float h_sd = h_ne - h_sw;\n"

        "       if (abs(h_md)<abs(h_sd)){ \n"
        "           ret.b = tex.g + 0.8333*(h_nw+h_se)*0.5 + 0.1667*(h_ne+h_sw)*0.5 ;\n"
        "       } else {\n"
        "           ret.b = tex.g + 0.1667*(h_nw+h_se)*0.5 + 0.8333*(h_ne+h_sw)*0.5 ;\n"
        "       }\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       float h_nw = mm.r - mm.g;\n"
        "       float h_sw = mp.r - mp.g;\n"
        "       float h_ne = pm.r - pm.g;\n"
        "       float h_se = pp.r - pp.g;\n"
        "       float h_md = h_nw - h_se;\n"
        "       float h_sd = h_ne - h_sw;\n"

        "       if (abs(h_md)<abs(h_sd)){ \n"
        "           ret.r = tex.g + 0.8333*(h_nw+h_se)*0.5 + 0.1667*(h_ne+h_sw)*0.5 ;\n"
        "       } else {\n"
        "           ret.r = tex.g + 0.1667*(h_nw+h_se)*0.5 + 0.8333*(h_ne+h_sw)*0.5 ;\n"
        "       }\n"
        "   }\n"
        "   return ret;\n"
        "}\n"

//----------------------------------------------------------------------------

        /*
         * Adaptive fuzzy color interpolation step 3
         */
        "vec4 fuzzy_demosaic_step3()\n"
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

        "   vec4 om =texture2D(texture, gl_TexCoord[0].st+vec2(0,-dy));\n" 
 
        "   vec4 mo =texture2D(texture, gl_TexCoord[0].st+vec2(-dx,0));\n" 
        "   vec4 tex =texture2D(texture, gl_TexCoord[0].st);\n" 
        "   vec4 po =texture2D(texture, gl_TexCoord[0].st+vec2(dx,0));\n" 
         
        "   vec4 op =texture2D(texture, gl_TexCoord[0].st+vec2(0,dy));\n"  
        
        //evaluate if the coordinates of the current pixel are even
        "   bool x_even = (mod(floor(gl_TexCoord[0].s/dx), 2.0) == 0.0);\n"
        "   bool y_even = (mod(floor(gl_TexCoord[0].t/dy), 2.0) == 0.0);\n"

            "   vec4 ret = tex;\n"

        //ret.g
        "   if ( ( !x_even && y_even ) || ( x_even && !y_even ) ){\n"

                //blue
        "       float h_n_b = om.b - om.g;\n"
        "       float h_s_b = op.b - op.g;\n"
        "       float h_e_b = po.b - po.g;\n"
        "       float h_w_b = mo.b - mo.g;\n"
        "       float h_hor_b = h_e_b - h_w_b;\n"
        "       float h_ver_b = h_n_b - h_s_b;\n"

        "       if (abs(h_hor_b)<abs(h_ver_b)){ \n"
        "           ret.b = tex.g + 0.8333*(h_w_b+h_e_b)*0.5 + 0.1667*(h_n_b+h_s_b)*0.5 ;\n"
        "       } else {\n"
        "           ret.b = tex.g + 0.1667*(h_w_b+h_e_b)*0.5 + 0.8333*(h_n_b+h_s_b)*0.5 ;\n"
        "       }\n"
                
                //red
        "       float h_n_r = om.r - om.g;\n"
        "       float h_s_r = op.r - op.g;\n"
        "       float h_e_r = po.r - po.g;\n"
        "       float h_w_r = mo.r - mo.g;\n"
        "       float h_hor_r = h_e_r - h_w_r;\n"
        "       float h_ver_r = h_n_r - h_s_r;\n"

        "       if (abs(h_hor_r)<abs(h_ver_r)){ \n"
        "           ret.r = tex.g + 0.8333*(h_w_r+h_e_r)*0.5 + 0.1667*(h_n_r+h_s_r)*0.5 ;\n"
        "       } else {\n"
        "           ret.r = tex.g + 0.1667*(h_w_r+h_e_r)*0.5 + 0.8333*(h_n_r+h_s_r)*0.5 ;\n"
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
        "      gl_FragColor.rgba = fuzzy_demosaic_step1().rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_1, step1_Program);

    string step2_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        "      gl_FragColor.rgba = fuzzy_demosaic_step2().rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_2, step2_Program);

    
    string step3_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        "      gl_FragColor.rgba = fuzzy_demosaic_step3().rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_3, step3_Program);
}


}
