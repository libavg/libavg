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

#ifndef _SVG_H_
#define _SVG_H_

#include "WrapPython.h"

#include "../api.h"

#include "../base/UTF8String.h"
#include "../graphics/Bitmap.h"
#include "BoostPython.h"
#include "SVGElement.h"

#include <librsvg/rsvg.h>
#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class Node;
typedef boost::shared_ptr<Node> NodePtr;

class SVG
{
public:
    SVG(const UTF8String& sFilename, bool bUnescapeIllustratorIDs=false);
    virtual ~SVG();

    BitmapPtr renderElement(const UTF8String& sElementID);
    BitmapPtr renderElement(const UTF8String& sElementID, const glm::vec2& size);
    BitmapPtr renderElement(const UTF8String& sElementID, float scale);
    NodePtr createImageNode(const UTF8String& sElementID,
            const py::dict& nodeAttrs);
    NodePtr createImageNode(const UTF8String& sElementID,
            const py::dict& nodeAttrs, const glm::vec2& renderSize);
    NodePtr createImageNode(const UTF8String& sElementID,
            const py::dict& nodeAttrs, float scale);
    glm::vec2 getElementPos(const UTF8String& sElementID);
    glm::vec2 getElementSize(const UTF8String& sElementID);

private:
    BitmapPtr internalRenderElement(const SVGElementPtr& pElement, 
        const glm::vec2& renderSize, const glm::vec2& size);
    NodePtr createImageNodeFromBitmap(BitmapPtr pBmp, 
            const py::dict& nodeAttrs);
    SVGElementPtr getElement(const UTF8String& sElementID);

    std::map<UTF8String, SVGElementPtr> m_ElementMap;
    UTF8String m_sFilename;
    bool m_bUnescapeIllustratorIDs;
    RsvgHandle* m_pRSVG;
};

}

#endif

