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

#include "GLConfig.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <string>

namespace avg {

using namespace std;

GLConfig::GLConfig()
{
}

GLConfig::GLConfig(bool bGLES, bool bUsePOTTextures, bool bUsePixelBuffers,
        int multiSampleSamples, ShaderUsage shaderUsage, bool bUseDebugContext)
    : m_bGLES(bGLES),
      m_bUsePOTTextures(bUsePOTTextures),
      m_bUsePixelBuffers(bUsePixelBuffers),
      m_MultiSampleSamples(multiSampleSamples),
      m_ShaderUsage(shaderUsage),
      m_bUseDebugContext(bUseDebugContext)
{
}

void GLConfig::log()
{
    AVG_TRACE(Logger::CONFIG, "  OpenGL flavor: " << (m_bGLES?"Mobile (ES)":"Desktop"));
    AVG_TRACE(Logger::CONFIG, "  Pixel buffers: " << (m_bUsePixelBuffers?"true":"false"));
    AVG_TRACE(Logger::CONFIG, "  Power of 2 textures: " <<
            (m_bUsePOTTextures?"true":"false"));
    if (m_MultiSampleSamples == 1) {
        AVG_TRACE(Logger::CONFIG, "  No multisampling");
    } else {
        AVG_TRACE(Logger::CONFIG, "  Multisampling with " << m_MultiSampleSamples 
                << " samples");
    }
    string sShader = shaderUsageToString(m_ShaderUsage);
    AVG_TRACE(Logger::CONFIG, "  Shader usage: " << sShader);
    AVG_TRACE(Logger::CONFIG, "  Debug context: " << (m_bUseDebugContext?"true":"false"));
}

std::string GLConfig::shaderUsageToString(ShaderUsage su)
{
    switch(su) {
        case FULL:
            return "full";
        case MINIMAL:
            return "minimal";
        case FRAGMENT_ONLY:
            return "fragment only";
        case AUTO:
            return "auto";
        default:
            AVG_ASSERT(false);
            return "";
    }
}

}
