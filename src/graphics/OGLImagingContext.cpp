//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "OGLImagingContext.h"
#undef check
#include "ShaderRegistry.h"

#include "../base/Exception.h"

#ifdef _WIN32
// XXX: If the following includes are not there, the MS linker optimizes away
// the classes so they can't be used by plugins anymore (!). Adding /OPT:NOREF
// to the linker flags doesn't help. 
#include "FilterGetAlpha.h"
#include "FilterErosion.h"
#include "FilterDilation.h"
#include "FilterGetAlpha.h"
#include "FilterResizeBilinear.h"
#include "FilterResizeGaussian.h"
#include "FilterThreshold.h"
#endif

#include <iostream>

namespace avg {

using namespace std;

OGLImagingContext::OGLImagingContext()
    : GLContext()
{
    init();

    if (!isSupported()) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                "GPU imaging not supported by current OpenGL configuration.");
    }

    setStandardState();
}

OGLImagingContext::~OGLImagingContext()
{
}

bool OGLImagingContext::isSupported()
{
    int glMajorVer;
    int glMinorVer;
    int slMajorVer;
    int slMinorVer;
    getGLVersion(glMajorVer, glMinorVer);
    getGLShadingLanguageVersion(slMajorVer, slMinorVer);
    return (glMajorVer > 1 && queryOGLExtension("GL_ARB_texture_rectangle") && 
            queryOGLExtension("GL_ARB_pixel_buffer_object") &&
            queryOGLExtension("GL_EXT_framebuffer_object"));
}

void OGLImagingContext::setStandardState()
{
    // Shading etc.
    glDisable(GL_BLEND);
    GLContext::getCurrent()->checkError("glDisable(GL_BLEND)");
    glShadeModel(GL_FLAT);
    GLContext::getCurrent()->checkError("glShadeModel(GL_FLAT)");
    glDisable(GL_DEPTH_TEST);
    GLContext::getCurrent()->checkError("glDisable(GL_DEPTH_TEST)");
    glDisable(GL_STENCIL_TEST);
    GLContext::getCurrent()->checkError("glDisable(GL_STENCIL_TEST)");

    // Texturing
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    GLContext::getCurrent()->checkError("glTexEnvf()");
    glBlendFunc(GL_ONE, GL_ZERO);
    GLContext::getCurrent()->checkError("glBlendFunc()");
    glDisable(GL_MULTISAMPLE);
    GLContext::getCurrent()->checkError("glDisable(GL_MULTISAMPLE);");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

}

