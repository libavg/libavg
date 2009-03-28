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

#ifndef _Node_H_
#define _Node_H_

#include "Event.h"

#include "../api.h"

#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <string>

namespace avg {

class Node;
class DivNode;
class AVGNode;
class ArgList;
class DisplayEngine;
class SDLDisplayEngine;
class AudioEngine;
class Player;
class NodeDefinition;
class Image;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::shared_ptr<DivNode> DivNodePtr;
typedef boost::weak_ptr<DivNode> DivNodeWeakPtr;
typedef boost::shared_ptr<AVGNode> AVGNodePtr;
typedef boost::weak_ptr<AVGNode> AVGNodeWeakPtr;
typedef boost::shared_ptr<Image> ImagePtr;

class AVG_API Node
{
    public:
        enum NodeState {NS_UNCONNECTED, NS_CONNECTED, NS_CANRENDER};
        
        template<class NodeType>
        static NodePtr buildNode(const ArgList& Args, bool bFromXML)
        {
            return NodePtr(new NodeType(Args, bFromXML));
        }
        static NodeDefinition createDefinition();
        
        virtual ~Node() = 0;
        virtual void setThis(NodeWeakPtr This, const NodeDefinition * pDefinition);
        virtual void setArgs(const ArgList& Args);
        virtual void setParent(DivNodeWeakPtr pParent, NodeState parentState);
        void removeParent();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect();
        virtual void disconnect();
        virtual void checkReload() {};

        virtual const std::string& getID() const;
        void setID(const std::string& ID);

        double getOpacity() const;
        void setOpacity(double opacity);
        
        bool getActive() const;
        void setActive(bool bActive);
        
        bool getSensitive() const;
        void setSensitive(bool bSensitive);

        DivNodePtr getParent() const;
        void unlink();

        void setMouseEventCapture();
        void releaseMouseEventCapture();
        void setEventCapture(int cursorID);
        void releaseEventCapture(int cursorID);
        void setEventHandler(Event::Type Type, int Sources, PyObject * pFunc);

        virtual NodePtr getElementByPos(const DPoint & pos);

        virtual void preRender();
        virtual void maybeRender(const DRect& Rect) {};
        virtual void render(const DRect& Rect) {};
        
        double getEffectiveOpacity();
        virtual std::string dump(int indent = 0);
        std::string getTypeStr() const;
        
        NodeState getState() const;

        bool operator ==(const Node& other) const;
        bool operator !=(const Node& other) const;

        long getHash() const;
        virtual const NodeDefinition* getDefinition() const;
        virtual bool handleEvent(EventPtr pEvent); 

    protected:
        Node();

        void addEventHandlers(Event::Type EventType, const std::string& Code);
        void addEventHandler(Event::Type EventType, Event::Source Source, 
                const std::string& Code);
        bool reactsToMouseEvents();
            
        SDLDisplayEngine * getDisplayEngine() const;
        AudioEngine * getAudioEngine() const;
        NodePtr getThis() const;
        void setState(NodeState State);
        void initFilename(std::string& sFilename);
        void checkReload(const std::string& sHRef, ImagePtr& pImage);

    private:
        PyObject * findPythonFunc(const std::string& Code);
        bool callPython(PyObject * pFunc, avg::EventPtr pEvent);

        struct EventHandlerID {
            EventHandlerID(Event::Type EventType, Event::Source Source);

            bool operator < (const EventHandlerID& other) const;

            Event::Type m_Type;
            Event::Source m_Source;
        };
        typedef std::map<EventHandlerID, PyObject *> EventHandlerMap;
        EventHandlerMap m_EventHandlerMap;

        DivNodeWeakPtr m_pParent;
        NodeWeakPtr m_This;
        SDLDisplayEngine * m_pDisplayEngine;
        AudioEngine * m_pAudioEngine;

        std::string m_ID;
        double m_Opacity;
        NodeState m_State;
        const NodeDefinition* m_pDefinition;

        bool m_bActive;
        bool m_bSensitive;
        double m_EffectiveOpacity;
};

}

#endif //_Node_H_
