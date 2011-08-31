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

#include "../base/Exception.h"

using namespace std;
using namespace boost;

namespace avg {
    
GLBufferCache::GLBufferCache()
{
}

GLBufferCache::~GLBufferCache()
{
}

unsigned int GLBufferCache::getBuffer()
{
    unsigned int bufferID;
    if (s_pGLBufferIDs.get() == 0) {
        s_pGLBufferIDs.reset(new vector<unsigned int>);
    }
    if (s_pGLBufferIDs->empty()) {
        glproc::GenBuffers(1, &bufferID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBO: GenBuffers()");
    } else {
        bufferID = s_pGLBufferIDs->back();
        s_pGLBufferIDs->pop_back();
    }
    return bufferID;
}

void GLBufferCache::returnBuffer(unsigned int bufferID)
{
    if (s_pGLBufferIDs.get() != 0) {
        s_pGLBufferIDs->push_back(bufferID);
    }
}

void GLBufferCache::deleteBuffers()
{
    if (s_pGLBufferIDs.get() != 0) {
        for (unsigned i=0; i<s_pGLBufferIDs->size(); ++i) {
            glproc::DeleteBuffers(1, &((*s_pGLBufferIDs)[i]));
        }
        s_pGLBufferIDs->clear();
        s_pGLBufferIDs.reset();
    }
}

}
