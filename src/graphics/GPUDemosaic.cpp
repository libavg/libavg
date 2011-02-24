

#include "GPUDemosaic.h"
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

GPUDemosaic::GPUDemosaic(const IntPoint& size, PixelFormat pfSrc, PixelFormat pfDest,
        bool bStandalone)
    : GPUFilter(size, pfSrc, pfDest, bStandalone, 2)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShaders();
}

GPUDemosaic::~GPUDemosaic()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUDemosaic::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step1_Shader = getShader(SHADERID_STEP_1);
    step1_Shader->activate();
    draw(pSrcTex);

    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    OGLShaderPtr step2_Shader = getShader(SHADERID_STEP_2);
    step2_Shader->activate();
    draw(getDestTex(0));

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    OGLShaderPtr step3_Shader = getShader(SHADERID_STEP_3);
    step3_Shader->activate();
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
        /*
         * Adaptive fuzzy color interpolation step 1
         */
        "float fuzzy_demosaic_step1()\n"
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


        "   float green = tex.g;\n"
        //Red
        "   if ( x_even && y_even ){ \n"
        "       float c_hor = 0.5*(-mmo.r + 2.0*mo.g - 2.0*po.g + ppo.r);\n"
        "       float c_ver = 0.5*(-omm.r + 2.0*om.g - 2.0*op.g + opp.r);\n"
        "       float i_hor = 0.5*(om.g + op.g) + 0.125*(-omm.r + 2.0*tex.r - opp.r);\n"
        "       float i_ver = 0.5*(mo.g + po.g) + 0.125*(-mmo.r + 2.0*tex.r - ppo.r);\n"
        "       if (abs(c_hor)<abs(c_ver)){ \n"
        "           green = 0.8333*i_hor+0.1667*i_ver;\n"
        "       } else {\n"
        "           if (abs(c_ver)<abs(c_hor)) { \n"
        "               green = 0.8333*i_ver+0.1667*i_hor;\n"
        "           } else {\n"
        "               green = 0.5*i_ver+0.5*i_hor;\n"
        "           }\n"
        "       }\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       float c_hor = 0.5*(-mmo.b + 2.0*mo.g - 2.0*po.g + ppo.b);\n"
        "       float c_ver = 0.5*(-omm.b + 2.0*om.g - 2.0*op.g + opp.b);\n"
        "       float i_hor = 0.5*(om.g + op.g) + 0.125*(-omm.b + 2.0*tex.b - opp.b);\n"
        "       float i_ver = 0.5*(mo.g + po.g) + 0.125*(-mmo.b + 2.0*tex.b - ppo.b);\n"
        "       if (abs(c_hor)<abs(c_ver)){ \n"
        "           green = 0.8333*i_hor+0.1667*i_ver;\n"
        "       } else {\n"
        "           if (abs(c_ver)<abs(c_hor)) { \n"
        "               green = 0.8333*i_ver+0.1667*i_hor;\n"
        "           } else {\n"
        "               green = 0.5*i_ver+0.5*i_hor;\n"
        "           }\n"
        "       }\n"
        "   }\n"
        "   return green;\n"
        "}\n"



        /*
         * Adaptive fuzzy color interpolation step 2
         */
        "vec3 fuzzy_demosaic_step2()\n"
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

        "   vec3 blueANDred;\n"
        "   blueANDred.rb = tex.rb;\n"
        "   blueANDred.g = 0.0;\n"
        
        //Red
        "   if ( x_even && y_even ){ \n"
        "       float h_nw = mm.b-mm.g;\n"
        "       float h_sw = mp.b-mp.g;\n"
        "       float h_ne = pm.b-pm.g;\n"
        "       float h_se = pp.b-pp.g;\n"
        "       float h_md = h_nw-h_se;\n"
        "       float h_sd = h_ne-h_sw;\n"

        "       if (abs(h_md)<abs(h_sd)){ \n"
        "           blueANDred.b = tex.g + 0.8333*(h_nw+h_se)*0.5 + 0.1667*(h_ne+h_sw)*0.5 ;\n"
        "       } else {\n"
        "           blueANDred.b = tex.g + 0.1667*(h_nw+h_se)*0.5 + 0.8333*(h_ne+h_sw)*0.5 ;\n"
        "       }\n"
        "   }\n"
            
        //Blue
        "   if ( !x_even && !y_even ){\n"
        "       float h_nw = mm.r-mm.g;\n"
        "       float h_sw = mp.r-mp.g;\n"
        "       float h_ne = pm.r-pm.g;\n"
        "       float h_se = pp.r-pp.g;\n"
        "       float h_md = h_nw-h_se;\n"
        "       float h_sd = h_ne-h_sw;\n"

        "       if (abs(h_md)<abs(h_sd)){ \n"
        "           blueANDred.r = tex.g + 0.8333*(h_nw+h_se)*0.5 + 0.1667*(h_ne+h_sw)*0.5 ;\n"
        "       } else {\n"
        "           blueANDred.r = tex.g + 0.1667*(h_nw+h_se)*0.5 + 0.8333*(h_ne+h_sw)*0.5 ;\n"
        "       }\n"
        "   }\n"
        "   return blueANDred;\n"
        "}\n"

        /*
         * Adaptive fuzzy color interpolation step 3
         */
        "vec3 fuzzy_demosaic_step3()\n"
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

        "   vec3 blueANDred;\n"
        "   blueANDred.rb = tex.rb;\n"
        "   blueANDred.g = 0.0;\n"

        //Green
        "   if ( ( !x_even && y_even ) || ( x_even && !y_even ) ){\n"

                //blue
        "       float h_n_b = om.b - om.g;\n"
        "       float h_s_b = op.b - op.g;\n"
        "       float h_e_b = po.b - po.g;\n"
        "       float h_w_b = mo.b - mo.g;\n"
        "       float h_hor_b = h_e_b - h_w_b;\n"
        "       float h_ver_b = h_n_b - h_s_b;\n"

        "       if (abs(h_hor_b)<abs(h_ver_b)){ \n"
        "           blueANDred.b = tex.g + 0.8333*(h_w_b+h_e_b)*0.5 + 0.1667*(h_n_b+h_s_b)*0.5 ;\n"
        "       } else {\n"
        "           blueANDred.b = tex.g + 0.1667*(h_w_b+h_e_b)*0.5 + 0.8333*(h_n_b+h_s_b)*0.5 ;\n"
        "       }\n"
                
                //red
        "       float h_n_r = om.r - om.g;\n"
        "       float h_s_r = op.r - op.g;\n"
        "       float h_e_r = po.r - po.g;\n"
        "       float h_w_r = mo.r - mo.g;\n"
        "       float h_hor_r = h_e_r - h_w_r;\n"
        "       float h_ver_r = h_n_r - h_s_r;\n"

        "       if (abs(h_hor_r)<abs(h_ver_r)){ \n"
        "           blueANDred.r = tex.g + 0.8333*(h_w_r+h_e_r)*0.5 + 0.1667*(h_n_r+h_s_r)*0.5 ;\n"
        "       } else {\n"
        "           blueANDred.r = tex.g + 0.1667*(h_w_r+h_e_r)*0.5 + 0.8333*(h_n_r+h_s_r)*0.5 ;\n"
        "       }\n"
        "   }\n"

        "   return blueANDred;\n"
        "}\n"



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
        //"      gl_FragColor.g = fuzzy_demosaic_step1();\n"
        //"      gl_FragColor.rba = texture2D(texture, gl_TexCoord[0].st).rba;\n"
        "      gl_FragColor.rgba = bilin_demosaic();\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_1, step1_Program);

    string step2_Program = sProgramHead +
        "void main(void)\n"
        "{\n"

        //"      gl_FragColor.rb = fuzzy_demosaic_step2().rb;\n"
        //"      gl_FragColor.ga = texture2D(texture, gl_TexCoord[0].st).ga;\n"
        "      gl_FragColor.rgba = texture2D(texture, gl_TexCoord[0].st).rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_2, step2_Program);

    
    string step3_Program = sProgramHead +
        "void main(void)\n"
        "{\n"
        //"      gl_FragColor.rb = fuzzy_demosaic_step3().rb;\n"
        //"      gl_FragColor.ga = texture2D(texture, gl_TexCoord[0].st).ga;\n"
        "      gl_FragColor.rgba = texture2D(texture, gl_TexCoord[0].st).rgba;\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_STEP_3, step3_Program);
}


}
