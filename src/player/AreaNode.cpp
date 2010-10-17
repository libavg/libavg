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

#include "AreaNode.h"

#include "Event.h"
#include "CursorEvent.h"
#include "MouseEvent.h"
#include "DivNode.h"
#include "SDLDisplayEngine.h"
#include "ArgList.h"
#include "NodeDefinition.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include <object.h>
#include <compile.h>
#include <eval.h>
#include "BoostPython.h"

#include <iostream>

using namespace boost;
using namespace boost::python;

using namespace std;

namespace avg {

NodeDefinition AreaNode::createDefinition()
{
    return NodeDefinition("areanode")
        .extendDefinition(VisibleNode::createDefinition())
        .addArg(Arg<double>("x", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.x)))
        .addArg(Arg<double>("y", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.y)))
        .addArg(Arg<DPoint>("pos", DPoint(0.0, 0.0)))
        .addArg(Arg<double>("width", 0.0, false, offsetof(AreaNode, m_UserSize.x)))
        .addArg(Arg<double>("height", 0.0, false, offsetof(AreaNode, m_UserSize.y)))
        .addArg(Arg<DPoint>("size", DPoint(0.0, 0.0)))
        .addArg(Arg<double>("angle", 0.0, false, offsetof(AreaNode, m_Angle)))
        .addArg(Arg<DPoint>("pivot", DPoint(-32767, -32767), false, 
                offsetof(AreaNode, m_Pivot)));
}

AreaNode::AreaNode()
    : m_RelViewport(0,0,0,0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AreaNode::~AreaNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void AreaNode::setArgs(const ArgList& args)
{
    VisibleNode::setArgs(args);
    args.getOverlayedArgVal(&m_RelViewport.tl, "pos", "x", "y", getID());
    args.getOverlayedArgVal(&m_UserSize, "size", "width", "height", getID());
    m_RelViewport.setWidth(m_UserSize.x);
    m_RelViewport.setHeight(m_UserSize.y);
}

void AreaNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));
    IntPoint MediaSize = getMediaSize();
    if (m_UserSize.x == 0.0) {
        m_RelViewport.setWidth(MediaSize.x);
    } else {
        m_RelViewport.setWidth(m_UserSize.x);
    }
    if (m_UserSize.y == 0.0) {
        m_RelViewport.setHeight(MediaSize.y);
    } else {
        m_RelViewport.setHeight(m_UserSize.y);
    }
    VisibleNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

double AreaNode::getX() const 
{
    return m_RelViewport.tl.x;
}

void AreaNode::setX(double x) 
{
    setViewport(x, -32767, -32767, -32767);
}

double AreaNode::getY() const 
{
    return m_RelViewport.tl.y;
}

void AreaNode::setY(double y) 
{
    setViewport(-32767, y, -32767, -32767);
}

const DPoint& AreaNode::getPos() const
{
    return m_RelViewport.tl;
}

void AreaNode::setPos(const DPoint& pt)
{
    setViewport(pt.x, pt.y, -32767, -32767);
}

double AreaNode::getWidth() 
{
    return getRelViewport().width();
}

void AreaNode::setWidth(double width) 
{
    m_UserSize.x = width;
    setViewport(-32767, -32767, -32767, -32767);
}

double AreaNode::getHeight()
{
    return getRelViewport().height();
}

void AreaNode::setHeight(double height) 
{
    m_UserSize.y = height;
    setViewport(-32767, -32767, -32767, -32767);
}

DPoint AreaNode::getSize() const
{
    return getRelViewport().size();
}

void AreaNode::setSize(const DPoint& pt)
{
    m_UserSize = pt;
    setViewport(-32767, -32767, -32767, -32767);
}

double AreaNode::getAngle() const
{
    return m_Angle;
}

void AreaNode::setAngle(double angle)
{
    m_Angle = fmod(angle, 2*PI);
}

DPoint AreaNode::getPivot() const
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        return getSize()/2;
    }
}

void AreaNode::setPivot(const DPoint& pt)
{
    m_Pivot.x = pt.x;
    m_Pivot.y = pt.y;
    m_bHasCustomPivot = true;
}

DPoint AreaNode::toLocal(const DPoint& globalPos) const
{
    DPoint localPos = globalPos-m_RelViewport.tl;
    return localPos.getRotatedPivot(-getAngle(), getPivot());
}

DPoint AreaNode::toGlobal(const DPoint& localPos) const
{
    DPoint globalPos = localPos.getRotatedPivot(getAngle(), getPivot());
    return globalPos+m_RelViewport.tl;
}

void AreaNode::getElementsByPos(const DPoint& pos, 
                vector<VisibleNodeWeakPtr>& pElements)
{
    if (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y &&
            reactsToMouseEvents())
    {
        pElements.push_back(getVThis());
    }
}

void AreaNode::maybeRender(const DRect& rect)
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
            SDLDisplayEngine * pEngine = getDisplayEngine();
            pEngine->pushTransform(getRelViewport().tl, getAngle(), getPivot());
            render(rect);
            pEngine->popTransform();
        }
    }
}

void AreaNode::setViewport(double x, double y, double width, double height)
{
    if (x == -32767) {
        x = getRelViewport().tl.x;
    }
    if (y == -32767) {
        y = getRelViewport().tl.y;
    }
    IntPoint MediaSize = getMediaSize();
    if (width == -32767) {
        if (m_UserSize.x == 0.0) {
            width = MediaSize.x;
        } else {
            width = m_UserSize.x;
        } 
    }
    if (height == -32767) {
        if (m_UserSize.y == 0.0) {
            height = MediaSize.y;
        } else {
            height = m_UserSize.y;
        } 
    }
    m_RelViewport = DRect (x, y, x+width, y+height);
}

const DRect& AreaNode::getRelViewport() const
{
//    cerr << "Node " << getID() << ": " << m_RelViewport << endl;
    return m_RelViewport;
}

string AreaNode::dump(int indent)
{
    string dumpStr = VisibleNode::dump(indent); 
    char sz[256];
    sprintf (sz, ", x=%.1f, y=%.1f, width=%.1f, height=%.1f\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y,
            m_RelViewport.width(), m_RelViewport.height());
    dumpStr += sz;

    return dumpStr; 
}

DPoint AreaNode::getUserSize() const
{
    return m_UserSize;
}

}
