//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _Node_H_
#define _Node_H_

#include "Region.h"
#include "ISurface.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <libxml/parser.h>

#include <vector>
#include <string>

namespace avg {

class DivNode;
class Event;
class Region;
class DisplayEngine;
class Player;
class MouseEvent;
class OGLSurface;

class Node
{
    public:
        enum NodeState {NS_UNCONNECTED, NS_CONNECTED, NS_DISABLED};
        
        virtual ~Node () = 0;
        virtual void connect(DisplayEngine * pEngine, DivNode * pParent);
        
        /**
         * Returns the unique id that can be used to reference the node.
         */
        virtual const std::string& getID () const;

        double getX() const;
        void setX(double x);
        
        double getY() const;
        void setY(double Y);
        
        double getWidth() const;
        void setWidth(double width);
        
        double getHeight() const;
        void setHeight(double height);
        
        double getOpacity() const;
        void setOpacity(double opacity);
        
        bool getActive() const;
        void setActive(bool bActive);
        
        bool getSensitive() const;
        void setSensitive(bool bSensitive);

        virtual DivNode * getParent() const;

        bool isActive();
        bool reactsToMouseEvents();
        virtual Node * getElementByPos (const DPoint & pos);
        virtual void prepareRender (int time, const DRect& parent);
        virtual void maybeRender (const DRect& Rect);
        virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int Child);
        virtual void addDirtyRect(const DRect& Rect);
        virtual void getDirtyRegion (Region& Region);
        virtual void setViewport (double x, double y, double width, 
                double height);
        virtual const DRect& getRelViewport () const;
        virtual const DRect& getAbsViewport () const;
        DRect getVisibleRect();
        virtual double getEffectiveOpacity();

        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
        void setParent(DivNode * pParent);
        
        virtual void handleMouseEvent (MouseEvent* pEvent); 
        virtual void invalidate();
        NodeState getState();
        
        // TODO: Do we still need this? Isn't rtti good enough?
        enum {NT_UNKNOWN, NT_IMAGE, NT_AVG, NT_VIDEO, NT_TEXT, NT_EXCL, 
                NT_CAMERA, NT_DIV, NT_PANOIMAGE};

    protected:
        Node ();
        Node (const xmlNodePtr xmlNode, Player * pPlayer);
        virtual DPoint getPreferredMediaSize() 
            { return DPoint(0,0); };
        Player * getPlayer();
        DisplayEngine * getEngine();

        void callPython (const std::string& Code, const avg::Event& Event);
            
        void initFilename (Player * pPlayer, std::string& sFilename);
        bool isInitialized ();
        void setState(NodeState State);
 
    private:
        void calcAbsViewport();

        DivNode * m_pParent;
        DisplayEngine * m_pEngine;
        Player * m_pPlayer;

        std::string m_ID;
        std::string m_MouseMoveHandler;
        std::string m_MouseButtonUpHandler;
        std::string m_MouseButtonDownHandler;
        std::string m_MouseOverHandler;
        std::string m_MouseOutHandler;

        DRect m_RelViewport;      // In coordinates relative to the parent.
        DRect m_AbsViewport;      // In window coordinates.
        double m_Opacity;
        bool m_bActive;
        bool m_bSensitive;
        
        // Initialization helpers.
        bool m_bInitialized;
        double m_InitialWidth;
        double m_InitialHeight;

        Region m_DirtyRegion;
        NodeState m_State;
};

}

#endif //_Node_H_

