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

#include "SVG.h"

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"
#include "../base/Logger.h"

#include "../graphics/PixelFormat.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Filterfliprgb.h"
#include "../graphics/FilterUnmultiplyAlpha.h"
#include "../graphics/BitmapLoader.h"

#include "OGLSurface.h"
#include "Player.h"
#include "ImageNode.h"

#include <glib-object.h>

#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif

#include <cairo.h>

#include <iostream>

using namespace std;


namespace avg {

SVG::SVG(const UTF8String& sFilename, bool bUnescapeIllustratorIDs)
    : m_sFilename(sFilename),
      m_bUnescapeIllustratorIDs(bUnescapeIllustratorIDs)
{
    GError* pErr = 0;
    m_pRSVG = rsvg_handle_new_from_file(m_sFilename.c_str(), &pErr);
    if (!m_pRSVG) {
        throw Exception(AVG_ERR_INVALID_ARGS, 
                string("Could not open svg file: ") + m_sFilename);
        g_error_free(pErr);
    }
}

SVG::~SVG()
{
    g_object_unref(m_pRSVG);
}

BitmapPtr SVG::renderElement(const UTF8String& sElementID)
{
    return renderElement(sElementID, 1);
}

BitmapPtr SVG::renderElement(const UTF8String& sElementID, const glm::vec2& size)
{
    SVGElementPtr pElement = getElement(sElementID);
    glm::vec2 elementSize = pElement->getSize();
    return internalRenderElement(pElement, size, elementSize);
}

BitmapPtr SVG::renderElement(const UTF8String& sElementID, float scale)
{
    SVGElementPtr pElement = getElement(sElementID);
    glm::vec2 size = pElement->getSize();
    glm::vec2 renderSize = size * scale;
    return internalRenderElement(pElement, renderSize, size);
}

NodePtr SVG::createImageNode(const UTF8String& sElementID, const py::dict& nodeAttrs)
{
    BitmapPtr pBmp = renderElement(sElementID);
    return createImageNodeFromBitmap(pBmp, nodeAttrs);
}

NodePtr SVG::createImageNode(const UTF8String& sElementID, const py::dict& nodeAttrs, 
        const glm::vec2& renderSize)
{
    BitmapPtr pBmp = renderElement(sElementID, renderSize);
    return createImageNodeFromBitmap(pBmp, nodeAttrs);
}

NodePtr SVG::createImageNode(const UTF8String& sElementID, const py::dict& nodeAttrs, 
        float scale)
{
    BitmapPtr pBmp = renderElement(sElementID, scale);
    return createImageNodeFromBitmap(pBmp, nodeAttrs);
}

glm::vec2 SVG::getElementPos(const UTF8String& sElementID)
{
    SVGElementPtr pElement = getElement(sElementID);
    return pElement->getPos();
}

glm::vec2 SVG::getElementSize(const UTF8String& sElementID)
{
    SVGElementPtr pElement = getElement(sElementID);
    return pElement->getSize();
}

BitmapPtr SVG::internalRenderElement(const SVGElementPtr& pElement, 
        const glm::vec2& renderSize, const glm::vec2& size)
{
    glm::vec2 pos = pElement->getPos();
    glm::vec2 scale(renderSize.x/size.x, renderSize.y/size.y);
    IntPoint boundingBox = IntPoint(renderSize) + 
            IntPoint(int(scale.x+0.5), int(scale.y+0.5));
    BitmapPtr pBmp(new Bitmap(boundingBox, B8G8R8A8));
    FilterFill<Pixel32>(Pixel32(0,0,0,0)).applyInPlace(pBmp);

    cairo_surface_t* pSurface;
    cairo_t* pCairo;
    pSurface = cairo_image_surface_create_for_data(pBmp->getPixels(), 
            CAIRO_FORMAT_ARGB32, boundingBox.x, boundingBox.y, 
            pBmp->getStride());
    pCairo = cairo_create(pSurface);
    cairo_scale(pCairo, scale.x, scale.y);
    cairo_translate(pCairo, -pos.x, -pos.y);
    rsvg_handle_render_cairo_sub(m_pRSVG, pCairo, pElement->getUnescapedID().c_str()); 

    FilterUnmultiplyAlpha().applyInPlace(pBmp);

    cairo_surface_destroy(pSurface);
    cairo_destroy(pCairo);
   
    if (!BitmapLoader::get()->isBlueFirst()) {
        FilterFlipRGB().applyInPlace(pBmp);
    }

    return pBmp;
}

NodePtr SVG::createImageNodeFromBitmap(BitmapPtr pBmp, const py::dict& nodeAttrs)
{
    ImageNodePtr pNode = boost::dynamic_pointer_cast<ImageNode>(
            Player::get()->createNode("image", nodeAttrs));
    pNode->setBitmap(pBmp);
    return pNode;
}

SVGElementPtr SVG::getElement(const UTF8String& sElementID)
{
    map<UTF8String, SVGElementPtr>::iterator pos = m_ElementMap.find(sElementID);
    if (pos == m_ElementMap.end()) {
        SVGElementPtr pElement(new SVGElement(m_pRSVG, m_sFilename, sElementID, 
                m_bUnescapeIllustratorIDs));
        m_ElementMap[sElementID] = pElement;
        return pElement;
    } else {
        return pos->second;
    }
}

}

