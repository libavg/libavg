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

#ifndef _AreaNode_H_
#define _AreaNode_H_

#include "Node.h"

#include "Event.h"
#include "ISurface.h"

#include "../base/Point.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <string>
#include <map>

namespace avg {

class AreaNode;
class DivNode;
class ArgList;

typedef boost::shared_ptr<AreaNode> AreaNodePtr;
typedef boost::weak_ptr<AreaNode> AreaNodeWeakPtr;
typedef boost::shared_ptr<DivNode> DivNodePtr;
typedef boost::weak_ptr<DivNode> DivNodeWeakPtr;

class AreaNode: public Node
{
    public:
        template<class NodeType>
        static NodePtr buildNode(const ArgList& Args, bool bFromXML)
        {
            return NodePtr(new NodeType(Args, bFromXML));
        }
        static NodeDefinition createDefinition();
        
        virtual ~AreaNode() = 0;
        virtual void setArgs(const ArgList& Args);
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        
        virtual DivNodePtr getDivParent() const;

        double getX() const;
        void setX(double x);
        
        double getY() const;
        void setY(double Y);

        const DPoint& getPos() const;
        void setPos(const DPoint& pt);

        virtual double getWidth();
        void setWidth(double width);
        
        virtual double getHeight();
        void setHeight(double height);
       
        DPoint getSize() const;
        void setSize(const DPoint& pt);

        double getAngle() const;
        void setAngle(double Angle);
        
        double getPivotX() const;
        void setPivotX(double Pivotx);
        
        double getPivotY() const;
        void setPivotY(double Pivoty);
        
        double getOpacity() const;
        void setOpacity(double opacity);
        
        bool getActive() const;
        void setActive(bool bActive);
        
        bool getSensitive() const;
        void setSensitive(bool bSensitive);

        DPoint getRelPos(const DPoint& AbsPos) const;
        DPoint getAbsPos(const DPoint& RelPos) const;

        void setMouseEventCapture();
        void releaseMouseEventCapture();
        void setEventCapture(int cursorID);
        void releaseEventCapture(int cursorID);
        void setEventHandler(Event::Type Type, int Sources, PyObject * pFunc);

        bool isActive();
        bool reactsToMouseEvents();
        virtual AreaNodePtr getElementByPos(const DPoint & pos);
        virtual void maybeRender(const DRect& Rect);
        virtual void setViewport(double x, double y, double width, double height);
        virtual const DRect& getRelViewport() const;
        virtual double getEffectiveOpacity();

        virtual std::string dump(int indent = 0);
        
        virtual void handleEvent(EventPtr pEvent); 
        virtual void checkReload() {};

        virtual IntPoint getMediaSize() 
            { return IntPoint(0,0); };

    protected:
        AreaNode();
        DPoint getPivot() const;

        void callPython(PyObject * pFunc, avg::EventPtr pEvent);
        void addEventHandlers(Event::Type EventType, const std::string& Code);
        void addEventHandler(Event::Type EventType, Event::Source Source, 
                const std::string& Code);
            
        void initFilename(std::string& sFilename);
        DPoint toLocal(const DPoint& pos) const;
        DPoint toGlobal(const DPoint& pos) const;
 
    private:
        PyObject * findPythonFunc(const std::string& Code);

        struct EventHandlerID {
            EventHandlerID(Event::Type EventType, Event::Source Source);

            bool operator < (const EventHandlerID& other) const;

            Event::Type m_Type;
            Event::Source m_Source;
        };
        typedef std::map<EventHandlerID, PyObject *> EventHandlerMap;
        EventHandlerMap m_EventHandlerMap;

        DRect m_RelViewport;      // In coordinates relative to the parent.
        double m_Opacity;
        bool m_bActive;
        bool m_bSensitive;
        double m_Angle;
        DPoint m_Pivot;
        bool m_bHasCustomPivot;
        
        // Size specified by user.
        DPoint m_WantedSize;
};

}

#endif
