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

#ifndef _SVGElement_H_
#define _SVGElement_H_

#include "../api.h"

#include "../base/UTF8String.h"
#include "../base/GLMHelper.h"

#include <librsvg/rsvg.h>
#include <boost/shared_ptr.hpp>

#include <map>

namespace avg {

class SVGElement
{
public:
    SVGElement(RsvgHandle* pRSVG, const UTF8String& sFilename,
            const UTF8String& sElementID, bool bUnescapeIllustratorIDs);

    const UTF8String& getUnescapedID() const;
    const glm::vec2& getPos() const;
    const glm::vec2& getSize() const;

private:
    UTF8String unescapeID(RsvgHandle* pRSVG, const UTF8String& sFilename, 
            const UTF8String& sElementID, bool bUnescapeIllustratorIDs);
    void throwIDNotFound(const UTF8String& sFilename, const UTF8String& sElementID);

    UTF8String m_sUnescapedID;
    glm::vec2 m_Pos;
    glm::vec2 m_Size;
};

typedef boost::shared_ptr<SVGElement> SVGElementPtr;

}

#endif

