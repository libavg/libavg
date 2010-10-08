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

#include "VectorNode.h"

#include "NodeDefinition.h"
#include "SDLDisplayEngine.h"
#include "OGLSurface.h"
#include "Image.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"
#include "../base/WideLine.h"
#include "../base/GeomHelper.h"
#include "../base/Triangle.h"
#include "../base/ObjectCounter.h"

#include "../graphics/VertexArray.h"
#include "../graphics/Filterfliprgb.h"

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition VectorNode::createDefinition()
{
    return NodeDefinition("vector")
        .extendDefinition(VisibleNode::createDefinition())
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(VectorNode, m_sColorName)))
        .addArg(Arg<double>("strokewidth", 1, false, offsetof(VectorNode, m_StrokeWidth)))
        .addArg(Arg<UTF8String>("texhref", "", false, offsetof(VectorNode, m_TexHRef)))
        .addArg(Arg<string>("blendmode", "blend", false, 
                offsetof(VectorNode, m_sBlendMode)))
        ;
}

VectorNode::VectorNode(const ArgList& args)
{
    m_pShape = ShapePtr(createDefaultShape());

    ObjectCounter::get()->incRef(&typeid(*this));
    m_TexHRef = args.getArgVal<UTF8String>("texhref"); 
    setTexHRef(m_TexHRef);
    m_sColorName = args.getArgVal<string>("color");
    m_Color = colorStringToColor(m_sColorName);
}

VectorNode::~VectorNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void VectorNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    setDrawNeeded();
    m_Color = colorStringToColor(m_sColorName);
    VisibleNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    m_pShape->moveToGPU(getDisplayEngine());
    m_OldOpacity = -1;
    setBlendModeStr(m_sBlendMode);
}

void VectorNode::connect(CanvasPtr pCanvas)
{
    VisibleNode::connect(pCanvas);
    checkReload();
}

void VectorNode::disconnect(bool bKill)
{
    if (bKill) {
        m_pShape->discard();
    } else {
        m_pShape->moveToCPU();
    }
    VisibleNode::disconnect(bKill);
}

void VectorNode::checkReload()
{
    VisibleNode::checkReload(m_TexHRef, m_pShape->getImage());
    if (getState() == VisibleNode::NS_CANRENDER) {
        m_pShape->moveToGPU(getDisplayEngine());
        setDrawNeeded();
    }
}

const UTF8String& VectorNode::getTexHRef() const
{
    return m_TexHRef;
}

void VectorNode::setTexHRef(const UTF8String& href)
{
    m_TexHRef = href;
    checkReload();
    setDrawNeeded();
}

void VectorNode::setBitmap(const Bitmap * pBmp)
{
    m_TexHRef = "";
    m_pShape->setBitmap(pBmp);
    setDrawNeeded();
}

const string& VectorNode::getBlendModeStr() const
{
    return m_sBlendMode;
}

void VectorNode::setBlendModeStr(const string& sBlendMode)
{
    m_sBlendMode = sBlendMode;
    m_BlendMode = DisplayEngine::stringToBlendMode(sBlendMode);
}

static ProfilingZoneID PrerenderProfilingZone("VectorNode::prerender");
static ProfilingZoneID VAProfilingZone("VectorNode::update VA");

void VectorNode::preRender()
{
    VisibleNode::preRender();
    ScopeTimer timer(PrerenderProfilingZone);
    double curOpacity = getEffectiveOpacity();

    VertexArrayPtr pVA = m_pShape->getVertexArray();
    {
        ScopeTimer timer(VAProfilingZone);
        if (m_bDrawNeeded || curOpacity != m_OldOpacity) {
            pVA->reset();
            Pixel32 color = getColorVal();
            color.setA((unsigned char)(curOpacity*255));
            calcVertexes(pVA, color);
            pVA->update();
            m_bDrawNeeded = false;
            m_OldOpacity = curOpacity;
        }
    }
    
}

void VectorNode::maybeRender(const DRect& rect)
{
    AVG_ASSERT(getState() == NS_CANRENDER);
    if (getActive()) {
        if (getEffectiveOpacity() > 0.01) {
            if (getID() != "") {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                        " with ID " << getID());
            } else {
                AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
            }
            getDisplayEngine()->setBlendMode(m_BlendMode);
            render(rect);
        }
    }
}

static ProfilingZoneID RenderProfilingZone("VectorNode::render");

void VectorNode::render(const DRect& rect)
{
    ScopeTimer timer(RenderProfilingZone);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    double curOpacity = getEffectiveOpacity();
    glColor4d(1.0, 1.0, 1.0, curOpacity);
    m_pShape->draw();
}

void VectorNode::setColor(const string& sColor)
{
    if (m_sColorName != sColor) {
        m_sColorName = sColor;
        m_Color = colorStringToColor(m_sColorName);
        m_bDrawNeeded = true;
    }
}

const string& VectorNode::getColor() const
{
    return m_sColorName;
}

void VectorNode::setStrokeWidth(double width)
{
    if (width != m_StrokeWidth) {
        m_bDrawNeeded = true;
        m_StrokeWidth = width;
    }
}

double VectorNode::getStrokeWidth() const
{
    return m_StrokeWidth;
}

Pixel32 VectorNode::getColorVal() const
{
    return m_Color;
}

DisplayEngine::BlendMode VectorNode::getBlendMode() const
{
    return m_BlendMode;
}

VectorNode::LineJoin VectorNode::string2LineJoin(const string& s)
{
    if (s == "miter") {
        return LJ_MITER;
    } else if (s == "bevel") {
        return LJ_BEVEL;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Vector linejoin "+s+" not supported."));
    }
}

string VectorNode::lineJoin2String(LineJoin lineJoin)
{
    switch(lineJoin) {
        case LJ_MITER:
            return "miter";
        case LJ_BEVEL:
            return "bevel";
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

void VectorNode::setDrawNeeded()
{
    m_bDrawNeeded = true;
}
        
bool VectorNode::isDrawNeeded()
{
    return m_bDrawNeeded;
}

void VectorNode::calcPolyLineCumulDist(vector<double>& cumulDists, 
        const vector<DPoint>& pts, bool bIsClosed)
{
    cumulDists.clear();
    cumulDists.reserve(pts.size());
    if (!pts.empty()) {
        vector<double> distances;
        distances.reserve(pts.size());
        double totalDist = 0;
        for (unsigned i = 1; i < pts.size(); ++i) {
            double dist = calcDist(pts[i], pts[i-1]);
            distances.push_back(dist);
            totalDist += dist;
        }
        if (bIsClosed) {
            double dist = calcDist(pts[pts.size()-1], pts[0]);
            distances.push_back(dist);
            totalDist += dist;
        }

        double cumulDist = 0;
        cumulDists.push_back(0);
        for (unsigned i = 0; i < distances.size(); ++i) {
            cumulDist += distances[i]/totalDist;
            cumulDists.push_back(cumulDist);
        }
    }
}

void VectorNode::calcEffPolyLineTexCoords(vector<double>& effTC, 
        const vector<double>& tc, const vector<double>& cumulDist)
{
    if (tc.empty()) {
        effTC = cumulDist;
    } else if (tc.size() == cumulDist.size()) {
        effTC = tc;
    } else {
        effTC.reserve(cumulDist.size());
        effTC = tc;
        double minGivenTexCoord = tc[0];
        double maxGivenTexCoord = tc[tc.size()-1];
        double maxCumulDist = cumulDist[tc.size()-1];
        int baselineDist = 0;
        for (unsigned i = tc.size(); i < cumulDist.size(); ++i) {
            int repeatFactor = int(cumulDist[i]/maxCumulDist);
            double effCumulDist = fmod(cumulDist[i], maxCumulDist);
            while (cumulDist[baselineDist+1] < effCumulDist) {
                baselineDist++;
            }
            double ratio = (effCumulDist-cumulDist[baselineDist])/
                    (cumulDist[baselineDist+1]-cumulDist[baselineDist]);
            double rawTexCoord = (1-ratio)*tc[baselineDist] +ratio*tc[baselineDist+1];
            double texCoord = rawTexCoord
                    +repeatFactor*(maxGivenTexCoord-minGivenTexCoord);
            effTC.push_back(texCoord);
        }
    }
}

void VectorNode::calcPolyLine(const vector<DPoint>& origPts, 
        const vector<double>& origTexCoords, bool bIsClosed, LineJoin lineJoin, 
        VertexArrayPtr& pVertexArray, Pixel32 color)
{
    vector<DPoint> pts;
    pts.reserve(origPts.size());
    vector<double> texCoords;
    texCoords.reserve(origPts.size());

    pts.push_back(origPts[0]);
    texCoords.push_back(origTexCoords[0]);
    for (unsigned i = 1; i < origPts.size(); ++i) {
        if (calcDistSquared(origPts[i], origPts[i-1])>0.1) {
            pts.push_back(origPts[i]);
            texCoords.push_back(origTexCoords[i]);
        }
    }
    if (bIsClosed) {
        texCoords.push_back(origTexCoords[origTexCoords.size()-1]);
    }

    int numPts = pts.size();

    // Create array of wide lines.
    vector<WideLine> lines;
    lines.reserve(numPts-1);
    for (int i = 0; i < numPts-1; ++i) {
        lines.push_back(WideLine(pts[i], pts[i+1], m_StrokeWidth));
    }
    if (bIsClosed) {
        lines.push_back(WideLine(pts[numPts-1], pts[0], m_StrokeWidth));
    }
    // First points
    if (bIsClosed) {
        WideLine lastLine = lines[lines.size()-1];
        DPoint pli = getLineLineIntersection(lastLine.pl0, lastLine.dir, 
                lines[0].pl0, lines[0].dir);
        DPoint pri = getLineLineIntersection(lastLine.pr0, lastLine.dir, 
                lines[0].pr0, lines[0].dir);
        Triangle tri(lastLine.pl1, lines[0].pl0, pri);
        if (tri.isClockwise()) {
            if (!DLineSegment(lastLine.pr0, lastLine.pr1).isPointOver(pri) &&
                    !DLineSegment(lines[0].pr0, lines[0].pr1).isPointOver(pri))
            {
                pri = lines[0].pr1;
            }
        } else {
            if (!DLineSegment(lastLine.pl0, lastLine.pl1).isPointOver(pli) &&
                    !DLineSegment(lines[0].pl0, lines[0].pl1).isPointOver(pli))
            {
                pli = lines[0].pl1;
            }
        }

        double curTC = texCoords[0];
        switch (lineJoin) {
            case LJ_MITER:
                pVertexArray->appendPos(pli, DPoint(curTC,1), color);
                pVertexArray->appendPos(pri, DPoint(curTC,0), color);
                break;
            case LJ_BEVEL: {
                    if (tri.isClockwise()) {
                        pVertexArray->appendPos(lines[0].pl0, DPoint(curTC,1), color);
                        pVertexArray->appendPos(pri, DPoint(curTC,0), color);
                    } else {
                        pVertexArray->appendPos(pli, DPoint(curTC,1), color);
                        pVertexArray->appendPos(lines[0].pr0, DPoint(curTC,0), color);
                    }
                }
                break;
            default:
                AVG_ASSERT(false);
                break;
        }
    } else {
        pVertexArray->appendPos(lines[0].pl0, DPoint(texCoords[0],1), color);
        pVertexArray->appendPos(lines[0].pr0, DPoint(texCoords[0],0), color);
    }

    // All complete line segments
    unsigned numNormalSegments;
    if (bIsClosed) {
        numNormalSegments = pts.size();
    } else {
        numNormalSegments = pts.size()-2;
    }
    for (unsigned i = 0; i < numNormalSegments; ++i) {
        const WideLine* pLine1 = &(lines[i]);
        const WideLine* pLine2;
        if (i == pts.size()-1) {
            pLine2 = &(lines[0]);
        } else {
            pLine2 = &(lines[i+1]);
        }
        DPoint pli = getLineLineIntersection(pLine1->pl0, pLine1->dir, pLine2->pl0, pLine2->dir);
        DPoint pri = getLineLineIntersection(pLine1->pr0, pLine1->dir, pLine2->pr0, pLine2->dir);
        Triangle tri(pLine1->pl1, pLine2->pl0, pri);
        if (tri.isClockwise()) {
            if (!DLineSegment(pLine1->pr0, pLine1->pr1).isPointOver(pri) &&
                    !DLineSegment(pLine2->pr0, pLine2->pr1).isPointOver(pri))
            {
                pri = pLine2->pr1;
            }
        } else {
            if (!DLineSegment(pLine1->pl0, pLine1->pl1).isPointOver(pli) &&
                    !DLineSegment(pLine2->pl0, pLine2->pl1).isPointOver(pli))
            {
                pli = pLine2->pl1;
            }
        }

        int curVertex = pVertexArray->getCurVert();
        double curTC = texCoords[i+1];
        switch (lineJoin) {
            case LJ_MITER:
                pVertexArray->appendPos(pli, DPoint(curTC,1), color);
                pVertexArray->appendPos(pri, DPoint(curTC,0), color);
                pVertexArray->appendQuadIndexes(
                        curVertex-1, curVertex-2, curVertex+1, curVertex);
                break;
            case LJ_BEVEL:
                {
                    double TC0;
                    double TC1;
                    if (tri.isClockwise()) {
                        calcBevelTC(*pLine1, *pLine2, true, texCoords, i+1, TC0, TC1);
                        pVertexArray->appendPos(pLine1->pl1, DPoint(TC0,1), color);
                        pVertexArray->appendPos(pLine2->pl0, DPoint(TC1,1), color);
                        pVertexArray->appendPos(pri, DPoint(curTC,0), color);
                        pVertexArray->appendQuadIndexes(
                                curVertex-1, curVertex-2, curVertex+2, curVertex);
                        pVertexArray->appendTriIndexes(
                                curVertex, curVertex+1, curVertex+2);
                    } else {
                        calcBevelTC(*pLine1, *pLine2, false,  texCoords, i+1, TC0, TC1);
                        pVertexArray->appendPos(pLine1->pr1, DPoint(TC0,0), color);
                        pVertexArray->appendPos(pli, DPoint(curTC,1), color);
                        pVertexArray->appendPos(pLine2->pr0, DPoint(TC1,0), color);
                        pVertexArray->appendQuadIndexes(
                                curVertex-2, curVertex-1, curVertex+1, curVertex);
                        pVertexArray->appendTriIndexes(
                                curVertex, curVertex+1, curVertex+2);
                    }
                }
                break;
            default:
                AVG_ASSERT(false);
        }
    }

    // Last segment (PolyLine only)
    if (!bIsClosed) {
        int curVertex = pVertexArray->getCurVert();
        double curTC = texCoords[numPts-1];
        pVertexArray->appendPos(lines[numPts-2].pl1, DPoint(curTC,1), color);
        pVertexArray->appendPos(lines[numPts-2].pr1, DPoint(curTC,0), color);
        pVertexArray->appendQuadIndexes(curVertex-1, curVertex-2, curVertex+1, curVertex);
    }
}

void VectorNode::calcBevelTC(const WideLine& line1, const WideLine& line2, 
        bool bIsLeft, const vector<double>& texCoords, unsigned i, 
        double& TC0, double& TC1)
{
    double line1Len = line1.getLen();
    double line2Len = line2.getLen();
    double triLen;
    if (bIsLeft) {
        triLen = calcDist(line1.pl1, line2.pl0);
    } else {
        triLen = calcDist(line1.pr1, line2.pr0);
    }
    double ratio0 = line1Len/(line1Len+triLen/2);
    TC0 = (1-ratio0)*texCoords[i-1]+ratio0*texCoords[i];
    double nextTexCoord;
    if (i == texCoords.size()-1) {
        nextTexCoord = texCoords[i];
    } else {
        nextTexCoord = texCoords[i+1];
    }
    double ratio1 = line2Len/(line2Len+triLen/2);
    TC1 = ratio1*texCoords[i]+(1-ratio1)*nextTexCoord;
}

int VectorNode::getNumDifferentPts(const vector<DPoint>& pts)
{
    int numPts = pts.size();
    for (unsigned i=1; i<pts.size(); ++i) {
        if (calcDistSquared(pts[i], pts[i-1])<0.1) {
            numPts--;
        }
    }
    return numPts;
}

Shape* VectorNode::createDefaultShape() const
{
    return new Shape(MaterialInfo(GL_REPEAT, GL_CLAMP_TO_EDGE, false));
}

}
