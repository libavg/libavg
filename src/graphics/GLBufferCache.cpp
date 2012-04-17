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

#include "GLBufferCache.h"

#include "GLContext.h"

#include "../base/Exception.h"

using namespace std;
using namespace boost;

namespace avg {
    
GLBufferCache::GLBufferCache()
{
}

GLBufferCache::~GLBufferCache()
{
    deleteBuffers();
}

unsigned int GLBufferCache::getBuffer()
{
    unsigned int bufferID;
    if (m_BufferIDs.empty()) {
        glproc::GenBuffers(1, &bufferID);
        GLContext::getCurrent()->checkError("PBO: GenBuffers()");
    } else {
        bufferID = m_BufferIDs.back();
        m_BufferIDs.pop_back();
    }
    return bufferID;
}

void GLBufferCache::returnBuffer(unsigned int bufferID)
{
    m_BufferIDs.push_back(bufferID);
}

void GLBufferCache::deleteBuffers()
{
    for (unsigned i=0; i<m_BufferIDs.size(); ++i) {
        glproc::DeleteBuffers(1, &(m_BufferIDs[i]));
    }
    m_BufferIDs.clear();
}

}
