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

#include "FilledVectorNode.h"

#include "TypeDefinition.h"
#include "Image.h"
#include "DivNode.h"

#include "../base/ScopeTimer.h"
#include "../base/Logger.h"
#include "../base/Exception.h"

using namespace std;
using namespace boost;

namespace avg {

void FilledVectorNode::registerType()
{
    TypeDefinition def = TypeDefinition("filledvectornode", "vectornode")
        .addArg(Arg<UTF8String>("filltexhref", "", false, 
                offsetof(FilledVectorNode, m_FillTexHRef)))
        .addArg(Arg<float>("fillopacity", 0, false, 
                offsetof(FilledVectorNode, m_FillOpacity)))
        .addArg(Arg<string>("fillcolor", "FFFFFF", false, 
                offsetof(FilledVectorNode, m_sFillColorName)))
        .addArg(Arg<glm::vec2>("filltexcoord1", glm::vec2(0,0), false,
                offsetof(FilledVectorNode, m_FillTexCoord1)))
        .addArg(Arg<glm::vec2>("filltexcoord2", glm::vec2(1,1), false,
                offsetof(FilledVectorNode, m_FillTexCoord2)))
        ;
    TypeRegistry::get()->registerType(def);
}

FilledVectorNode::FilledVectorNode(const ArgList& args)
    : VectorNode(args),
      m_pFillShape(new Shape(MaterialInfo(GL_REPEAT, GL_REPEAT, false)))
{
    m_FillTexHRef = args.getArgVal<UTF8String>("filltexhref"); 
    setFillTexHRef(m_FillTexHRef);
    m_sFillColorName = args.getArgVal<string>("fillcolor");
    m_FillColor = colorStringToColor(m_sFillColorName);
}

FilledVectorNode::~FilledVectorNode()
{
}

void FilledVectorNode::connectDisplay()
{
    VectorNode::connectDisplay();
    m_FillColor = colorStringToColor(m_sFillColorName);
    m_pFillShape->moveToGPU();
    m_OldOpacity = -1;
}

void FilledVectorNode::disconnect(bool bKill)
{
    if (bKill) {
        m_pFillShape->discard();
    } else {
        m_pFillShape->moveToCPU();
    }
    VectorNode::disconnect(bKill);
}

void FilledVectorNode::checkReload()
{
    Node::checkReload(m_FillTexHRef, m_pFillShape->getImage());
    if (getState() == Node::NS_CANRENDER) {
        m_pFillShape->moveToGPU();
        setDrawNeeded();
    }
    VectorNode::checkReload();
}

const UTF8String& FilledVectorNode::getFillTexHRef() const
{
    return m_FillTexHRef;
}

void FilledVectorNode::setFillTexHRef(const UTF8String& href)
{
    m_FillTexHRef = href;
    checkReload();
    setDrawNeeded();
}

void FilledVectorNode::setFillBitmap(BitmapPtr pBmp)
{
    m_FillTexHRef = "";
    m_pFillShape->setBitmap(pBmp);
    setDrawNeeded();
}

const glm::vec2& FilledVectorNode::getFillTexCoord1() const
{
    return m_FillTexCoord1;
}

void FilledVectorNode::setFillTexCoord1(const glm::vec2& pt)
{
    m_FillTexCoord1 = pt;
    setDrawNeeded();
}

const glm::vec2& FilledVectorNode::getFillTexCoord2() const
{
    return m_FillTexCoord2;
}

void FilledVectorNode::setFillTexCoord2(const glm::vec2& pt)
{
    m_FillTexCoord2 = pt;
    setDrawNeeded();
}

float FilledVectorNode::getFillOpacity() const
{
    return m_FillOpacity;
}

void FilledVectorNode::setFillOpacity(float opacity)
{
    m_FillOpacity = opacity;
    setDrawNeeded();
}

void FilledVectorNode::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    float curOpacity = parentEffectiveOpacity*m_FillOpacity;

    VertexDataPtr pShapeVD = m_pFillShape->getVertexData();
    if (isDrawNeeded() || curOpacity != m_OldOpacity) {
        pShapeVD->reset();
        Pixel32 color = getFillColorVal();
        calcFillVertexes(pShapeVD, color);
        m_OldOpacity = curOpacity;
    }
    if (isVisible()) {
        m_pFillShape->setVertexArray(pVA);
    }
    VectorNode::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
}

static ProfilingZoneID RenderProfilingZone("FilledVectorNode::render");

void FilledVectorNode::render()
{
    ScopeTimer Timer(RenderProfilingZone);
    float curOpacity = getParent()->getEffectiveOpacity()*m_FillOpacity;
    if (curOpacity > 0.01) {
        m_pFillShape->draw(getTransform(), curOpacity);
    }
    VectorNode::render();
}

void FilledVectorNode::setFillColor(const string& sColor)
{
    if (m_sFillColorName != sColor) {
        m_sFillColorName = sColor;
        m_FillColor = colorStringToColor(m_sFillColorName);
        setDrawNeeded();
    }
}

const string& FilledVectorNode::getFillColor() const
{
    return m_sFillColorName;
}

Pixel32 FilledVectorNode::getFillColorVal() const
{
    return m_FillColor;
}

glm::vec2 FilledVectorNode::calcFillTexCoord(const glm::vec2& pt, const glm::vec2& minPt, 
        const glm::vec2& maxPt)
{
    glm::vec2 texPt;
    texPt.x = (m_FillTexCoord2.x-m_FillTexCoord1.x)*(pt.x-minPt.x)/(maxPt.x-minPt.x)
            +m_FillTexCoord1.x;
    texPt.y = (m_FillTexCoord2.y-m_FillTexCoord1.y)*(pt.y-minPt.y)/(maxPt.y-minPt.y)
            +m_FillTexCoord1.y;
    return texPt;
}

bool FilledVectorNode::isVisible() const
{
    return getEffectiveActive() && (getEffectiveOpacity() > 0.01 || 
            getParent()->getEffectiveOpacity()*m_FillOpacity > 0.01);
}

}
