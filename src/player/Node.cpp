//
// $Id$
// 

#include "Node.h"
#include "DivNode.h"

#include "Event.h"
#include "MouseEvent.h"
#include "Container.h"
#include "Player.h"
#include "IDisplayEngine.h"

#include "../JSScript.h"
#include "../JSFactoryBase.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <paintlib/plpoint.h>
#include <paintlib/plrect.h>

#include <iostream>

using namespace std;

namespace avg {

Node::Node ()
    : m_pParent(0),
      m_pPlayer(0),
      m_ID(""),
      m_MouseMoveHandler(""),
      m_MouseButtonUpHandler(""),
      m_MouseButtonDownHandler(""),
      m_MouseOverHandler(""),
      m_MouseOutHandler(""),
      m_RelViewport(0,0,0,0),
      m_AbsViewport(0,0,0,0),
      m_z(0),
      m_Opacity(1.0),
      m_bActive(true),
      m_bInitialized(false),
      m_InitialWidth(0),
      m_InitialHeight(0)
{
}

Node::~Node()
{
}

void Node::init(IDisplayEngine * pEngine, Container * pParent, 
        Player * pPlayer)
{
    m_pParent = pParent;
    m_pEngine = pEngine;
    m_pPlayer = pPlayer;
}

void Node::initVisible()
{
    DPoint PreferredSize = getPreferredMediaSize();
    
    if (m_InitialWidth == 0) {
        m_InitialWidth = (int)PreferredSize.x;
    }
    if (m_InitialHeight == 0) {
        m_InitialHeight = (int)PreferredSize.y;
    }
    m_RelViewport.SetWidth(m_InitialWidth);
    m_RelViewport.SetHeight(m_InitialHeight);
    DPoint pos(0,0);
    if (m_pParent) {
        pos = m_pParent->getAbsViewport().tl;
    } 
    m_AbsViewport = DRect (pos+getRelViewport().tl, pos+getRelViewport().br);
    m_bInitialized = true;
}

const string& Node::getID ()
{
    return m_ID;
}

Container* Node::getParent()
{
    return m_pParent;
}

void Node::setZ(int z)
{
    m_z = z;
    if (getParent()) {
        getParent()->zorderChange(this);
    }
    if (m_bActive) {
        invalidate();
    }
}

void Node::setActive(bool bActive)
{
    if (bActive != m_bActive) {
        m_bActive = bActive;
        invalidate();
    }
}

bool Node::isActive()
{
    return m_bActive;
}

Node * Node::getElementByPos (const DPoint & pos)
{
    if (getVisibleRect().Contains(pos) && m_bActive) {
        return this;
    } else {
        return 0;
    }
}

void Node::prepareRender (int time, const DRect& parent)
{
    calcAbsViewport();
}

void Node::maybeRender (const DRect& Rect)
{
    if (m_bActive) {
        bool bVisible;
        // FIXME
        if (dynamic_cast<DivNode*>(this) != 0) {
            bVisible = getEngine()->pushClipRect(getVisibleRect(), true);
        } else {
            bVisible = getEngine()->pushClipRect(getVisibleRect(), false);
        }
        if (bVisible) {
            if (getEffectiveOpacity() > 0.01) {
                if (!getParent() || 
                    !getParent()->obscures(getEngine()->getClipRect(), getZ()))
                {
                    if (m_ID != "") {
                        AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                                " with ID " << m_ID);
                    } else {
                        AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
                    }
                    render(Rect);
                } 
            }
        }
        getEngine()->popClipRect();
    }
}

void Node::render (const DRect& Rect)
{
}

bool Node::obscures (const DRect& Rect, int z)  
{
    return false;
}

void Node::addDirtyRect(const DRect& Rect)
{
    m_DirtyRegion.addRect(Rect);
}

void Node::getDirtyRegion (Region& Region)
{
    Region.addRegion(m_DirtyRegion);
    m_DirtyRegion.clear();
}

void Node::invalidate()
{
    if (m_bInitialized) {
        addDirtyRect(getVisibleRect());
    }
}

Player * Node::getPlayer()
{
    return m_pPlayer;
}

void Node::setViewport (double x, double y, double width, double height)
{
    if (m_bActive) {
        invalidate();
    }
    if (x == -32767) {
        x = getRelViewport().tl.x;
    }
    if (y == -32767) {
        y = getRelViewport().tl.y;
    }
    if (width == -32767) {
        width = getRelViewport().Width();
    }
    if (height == -32767) {
        height = getRelViewport().Height();
    }
    m_RelViewport = DRect (x, y, x+width, y+height);
    calcAbsViewport();
    if (m_bActive) {
        invalidate();
    }
}

const DRect& Node::getRelViewport ()
{
//    cerr << "Node " << m_ID << ": (" << m_RelViewport.tl.x << ", " 
//            << m_RelViewport.tl.y << ")" << endl;
    return m_RelViewport;
}

const DRect& Node::getAbsViewport ()
{
    return m_AbsViewport;
}

DRect Node::getVisibleRect()
{
    Node * pParent = getParent();
    DRect visRect = getAbsViewport();
    if (pParent) {
        DRect parent = getParent()->getVisibleRect();
        visRect.Intersect(parent);
    }
    return visRect;
}

void Node::calcAbsViewport()
{
    Node * pParent = getParent();
    if (pParent) {
        DRect parent = pParent->getAbsViewport();
        m_AbsViewport = DRect(parent.tl+getRelViewport().tl, 
                parent.tl+getRelViewport().br);
    } else {
        m_AbsViewport = getRelViewport();
    }
    if (m_AbsViewport.Width() < 0 || m_AbsViewport.Height() < 0) {
        m_AbsViewport.br=m_AbsViewport.tl;
    }
}

int Node::getZ ()
{
    return m_z;
}

double Node::getOpacity()
{
    return m_Opacity;
}

void Node::setOpacity(double o)
{
    m_Opacity = o;
    if (m_bActive) {
        invalidate();
    }
}

double Node::getEffectiveOpacity()
{
    if (getParent()) {
        return m_Opacity*getParent()->getEffectiveOpacity();
    } else {
        return m_Opacity;
    }
}

IDisplayEngine * Node::getEngine()
{
    return m_pEngine;
}

string Node::dump (int indent)
{
    string dumpStr = string(indent, ' ') + getTypeStr() + ": m_ID=" + m_ID;
    char sz[256];
    sprintf (sz, ", x=%.1f, y=%.1f, z=%i, width=%.1f, height=%.1f, opacity=%.2f\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y, m_z, 
            m_RelViewport.Width(), m_RelViewport.Height(), m_Opacity);
    dumpStr += sz;
    sprintf (sz, "        Abs: (x=%.1f, y=%.1f, width=%.1f, height=%.1f)\n",
            m_AbsViewport.tl.x, m_AbsViewport.tl.y,  
            m_AbsViewport.Width(), m_AbsViewport.Height());
    dumpStr += sz;

    return dumpStr; 
}

string Node::getTypeStr ()
{
    return "Node";
}


void Node::setParent(Container * pParent)
{
    if (m_pParent) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Can't change parent of node."));
    }
    m_pParent = pParent;
    JSFactoryBase::setParent(getJSPeer(), pParent);
}

void Node::handleMouseEvent (MouseEvent* pEvent, JSContext * pJSContext)
{
    string Code;
    pEvent->setElement(this);
    int EventType = pEvent->getType();
    switch (EventType) {
        case Event::MOUSEMOTION:
            Code = m_MouseMoveHandler;
            break;
        case Event::MOUSEBUTTONDOWN:
            Code = m_MouseButtonDownHandler;
            break;
        case Event::MOUSEBUTTONUP:
            Code = m_MouseButtonUpHandler;
            break;
        case Event::MOUSEOVER:
            Code = m_MouseOverHandler;
            break;
        case Event::MOUSEOUT:
            Code = m_MouseOutHandler;
            break;
         default:
            break;
    }
    if (!Code.empty()) {
        callJS(Code, pJSContext);
    }
    if (m_pParent) {
        m_pParent->handleMouseEvent (pEvent, pJSContext);
    }
}

void Node::callJS (const string& Code, JSContext * pJSContext)
{

    // TODO: precompile.
    JSScript Script(Code, "EventScript", 0, pJSContext);
    Script.run();
}

void Node::initFilename(Player * pPlayer, string& sFilename)
{
    if (sFilename[0] != '/') {
        sFilename = pPlayer->getCurDirName() + sFilename;
    }
}

bool Node::isInitialized()
{
    return m_bInitialized;
}

}

