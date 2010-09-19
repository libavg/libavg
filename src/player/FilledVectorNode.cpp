//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "NodeDefinition.h"
#include "Image.h"
#include "DivNode.h"

#include "../player/SDLDisplayEngine.h"
#include "../base/ScopeTimer.h"
#include "../base/Logger.h"
#include "../base/Exception.h"

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition FilledVectorNode::createDefinition()
{
    return NodeDefinition("filledvector")
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<UTF8String>("filltexhref", "", false, 
                offsetof(FilledVectorNode, m_FillTexHRef)))
        .addArg(Arg<double>("fillopacity", 0, false, 
                offsetof(FilledVectorNode, m_FillOpacity)))
        .addArg(Arg<string>("fillcolor", "FFFFFF", false, 
                offsetof(FilledVectorNode, m_sFillColorName)))
        .addArg(Arg<DPoint>("filltexcoord1", DPoint(0,0), false,
                offsetof(FilledVectorNode, m_FillTexCoord1)))
        .addArg(Arg<DPoint>("filltexcoord2", DPoint(1,1), false,
                offsetof(FilledVectorNode, m_FillTexCoord2)))
        ;
}

FilledVectorNode::FilledVectorNode(const ArgList& Args)
    : VectorNode(Args),
      m_pFillShape(new Shape(MaterialInfo(GL_REPEAT, GL_REPEAT, false)))
{
    m_FillTexHRef = Args.getArgVal<UTF8String>("filltexhref"); 
    setFillTexHRef(m_FillTexHRef);
    m_sFillColorName = Args.getArgVal<string>("fillcolor");
    m_FillColor = colorStringToColor(m_sFillColorName);
}

FilledVectorNode::~FilledVectorNode()
{
}

void FilledVectorNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    VectorNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    m_FillColor = colorStringToColor(m_sFillColorName);
    m_pFillShape->moveToGPU(getDisplayEngine());
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
    VisibleNode::checkReload(m_FillTexHRef, m_pFillShape->getImage());
    if (getState() == VisibleNode::NS_CANRENDER) {
        m_pFillShape->moveToGPU(getDisplayEngine());
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

void FilledVectorNode::setFillBitmap(const Bitmap * pBmp)
{
    m_FillTexHRef = "";
    m_pFillShape->setBitmap(pBmp);
    setDrawNeeded();
}

const DPoint& FilledVectorNode::getFillTexCoord1() const
{
    return m_FillTexCoord1;
}

void FilledVectorNode::setFillTexCoord1(const DPoint& pt)
{
    m_FillTexCoord1 = pt;
    setDrawNeeded();
}

const DPoint& FilledVectorNode::getFillTexCoord2() const
{
    return m_FillTexCoord2;
}

void FilledVectorNode::setFillTexCoord2(const DPoint& pt)
{
    m_FillTexCoord2 = pt;
    setDrawNeeded();
}

double FilledVectorNode::getFillOpacity() const
{
    return m_FillOpacity;
}

void FilledVectorNode::setFillOpacity(double opacity)
{
    m_FillOpacity = opacity;
    setDrawNeeded();
}

void FilledVectorNode::preRender()
{
    VisibleNode::preRender();
    double curOpacity = getDivParent()->getEffectiveOpacity()*m_FillOpacity;
    VertexArrayPtr pFillVA;
    pFillVA = m_pFillShape->getVertexArray();
    if (isDrawNeeded() || curOpacity != m_OldOpacity) {
        pFillVA->reset();
        Pixel32 color = getFillColorVal();
        color.setA((unsigned char)(curOpacity*255));
        calcFillVertexes(pFillVA, color);
        pFillVA->update();
        m_OldOpacity = curOpacity;
    }
    VectorNode::preRender();
}

void FilledVectorNode::maybeRender(const DRect& Rect)
{
    AVG_ASSERT(getState() == NS_CANRENDER);
    if (getActive()) {
        if (getEffectiveOpacity() > 0.01 || 
                getDivParent()->getEffectiveOpacity()*m_FillOpacity > 0.01)
        {
            if (getID() != "") {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                        " with ID " << getID());
            } else {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
            }
            getDisplayEngine()->setBlendMode(getBlendMode());
            render(Rect);
        }
    }
}

static ProfilingZoneID RenderProfilingZone("FilledVectorNode::render");

void FilledVectorNode::render(const DRect& rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    double curOpacity = getDivParent()->getEffectiveOpacity()*m_FillOpacity;
    if (curOpacity > 0.01) {
        glColor4d(1.0, 1.0, 1.0, curOpacity);
        m_pFillShape->draw();
    }
    VectorNode::render(rect);
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

DPoint FilledVectorNode::calcFillTexCoord(const DPoint& pt, const DPoint& minPt, 
        const DPoint& maxPt)
{
    DPoint texPt;
    texPt.x = (m_FillTexCoord2.x-m_FillTexCoord1.x)*(pt.x-minPt.x)/(maxPt.x-minPt.x)
            +m_FillTexCoord1.x;
    texPt.y = (m_FillTexCoord2.y-m_FillTexCoord1.y)*(pt.y-minPt.y)/(maxPt.y-minPt.y)
            +m_FillTexCoord1.y;
    return texPt;
}

}
