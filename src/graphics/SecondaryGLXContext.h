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
#ifndef _SecondaryGLXContext_H_
#define _SecondaryGLXContext_H_
#include "../api.h"

#include "GLXContext.h"
#include "../base/Rect.h"

#include "../base/Exception.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class AVG_API SecondaryGLXContext: public GLXContext
{
public:
    SecondaryGLXContext(const GLConfig& glConfig, const std::string& sDisplay,
            const IntRect& windowDimensions, bool bHasWindowFrame);
    virtual ~SecondaryGLXContext();

private:
    void createContext(GLConfig& glConfig, const std::string& sDisplay, 
            const IntRect& windowDimensions, bool bHasWindowFrame);

    ::Window m_Window;
};

typedef boost::shared_ptr<SecondaryGLXContext> SecondaryGLXContextPtr;

}
#endif


