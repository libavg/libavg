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

namespace avg {

GLConfig::GLConfig()
{
}

GLConfig::GLConfig(bool bUsePOTTextures, bool bUseShaders, bool bUsePixelBuffers,
            int multiSampleSamples)
    : m_bUsePOTTextures(bUsePOTTextures),
      m_bUsePixelBuffers(bUsePixelBuffers),
      m_MultiSampleSamples(multiSampleSamples)
{
}

void GLConfig::log()
{
    AVG_TRACE(Logger::CONFIG, "  Pixel buffers: " << (m_bUsePixelBuffers?"true":"false"));
    AVG_TRACE(Logger::CONFIG, "  Power of 2 textures: " <<
            (m_bUsePOTTextures?"true":"false"));
    if (m_MultiSampleSamples == 1) {
        AVG_TRACE(Logger::CONFIG, "  No multisampling");
    } else {
        AVG_TRACE(Logger::CONFIG, "  Multisampling with " << m_MultiSampleSamples 
                << " samples");
    }
}

}
