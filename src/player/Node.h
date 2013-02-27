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

#ifndef _Node_H_
#define _Node_H_

#include "../api.h"

#include "Publisher.h"
#include "Event.h"
#include "Image.h"

#include "../base/Rect.h"
#include "../graphics/Pixel32.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <string>
#include <vector>
#include <list>
#include <map>

namespace avg {

class Node;
typedef boost::shared_ptr<Node> NodePtr;
class DivNode;
typedef boost::shared_ptr<DivNode> DivNodePtr;
class ArgList;
class TypeDefinition;

class CanvasNode;
typedef boost::shared_ptr<CanvasNode> CanvasNodePtr;
class AVGNode;
typedef boost::shared_ptr<AVGNode> AVGNodePtr;
class Image;
typedef boost::shared_ptr<Image> ImagePtr;
class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;
class Canvas;
typedef boost::shared_ptr<Canvas> CanvasPtr;
typedef boost::weak_ptr<Canvas> CanvasWeakPtr;

class AVG_API Node: public Publisher
{
    public:
        enum NodeState {NS_UNCONNECTED, NS_CONNECTED, NS_CANRENDER};

        static void registerType();
        void registerInstance(PyObject* pSelf, const DivNodePtr& pParent);
        
        virtual ~Node();
        virtual void setParent(DivNode* pParent, NodeState parentState,
                CanvasPtr pCanvas);
        virtual void removeParent();
        void checkSetParentError(DivNode* pParent);
        DivNodePtr getParent() const;
        std::vector<NodePtr> getParentChain();

        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);
        void unlink(bool bKill=false);

        virtual void checkReload() {};

        virtual void setID(const std::string& ID);

        float getOpacity() const;
        void setOpacity(float opacity);
        
        bool getActive() const;
        void setActive(bool bActive);
        
        bool getSensitive() const;
        void setSensitive(bool bSensitive);

        void setMouseEventCapture();
        void releaseMouseEventCapture();
        void setEventCapture(int cursorID);
        void releaseEventCapture(int cursorID);
        void setEventHandler(Event::Type Type, int Sources, PyObject * pFunc);
        void connectEventHandler(Event::Type type, int sources, 
                PyObject * pObj, PyObject * pFunc);
        void disconnectEventHandler(PyObject * pObj, PyObject * pFunc=0);

        glm::vec2 getRelPos(const glm::vec2& absPos) const;
        glm::vec2 getAbsPos(const glm::vec2& relPos) const;
        virtual glm::vec2 toLocal(const glm::vec2& pos) const;
        virtual glm::vec2 toGlobal(const glm::vec2& pos) const;
        NodePtr getElementByPos(const glm::vec2& pos);
        virtual void getElementsByPos(const glm::vec2& pos, 
                std::vector<NodePtr>& pElements);

        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void maybeRender(const glm::mat4& parentTransform) {};
        virtual void render() {};
        virtual void renderOutlines(const VertexArrayPtr& pVA, Pixel32 color) {};

        float getEffectiveOpacity() const;
        virtual std::string dump(int indent = 0);
        
        NodeState getState() const;
        CanvasPtr getCanvas() const;

        virtual bool handleEvent(EventPtr pEvent); 

        virtual const std::string& getID() const;
    
    protected:
        Node(const std::string& sPublisherName="Node");

        bool reactsToMouseEvents();
            
        void setState(NodeState state);
        void initFilename(std::string& sFilename);
        bool checkReload(const std::string& sHRef, const ImagePtr& pImage,
                Image::TextureCompression comp = Image::TEXTURECOMPRESSION_NONE);
        virtual bool isVisible() const;
        bool getEffectiveActive() const;
        NodePtr getSharedThis();

    private:
        std::string m_ID;

        DivNode* m_pParent;

        struct EventID {
            EventID(Event::Type eventType, Event::Source source);

            bool operator < (const EventID& other) const;

            Event::Type m_Type;
            Event::Source m_Source;
        };

        struct EventHandler {
            EventHandler(PyObject * pObj, PyObject * pMethod);
            EventHandler(const EventHandler& other);
            ~EventHandler();

            PyObject * m_pObj;
            PyObject * m_pMethod;
        };

        typedef std::list<EventHandler> EventHandlerArray;
        typedef boost::shared_ptr<EventHandlerArray> EventHandlerArrayPtr;
        typedef std::map<EventID, EventHandlerArrayPtr> EventHandlerMap;

        void connectOneEventHandler(const EventID& id, PyObject * pObj, PyObject * pFunc);
        void dumpEventHandlers();
        std::string getEventMessageID(const EventPtr& pEvent);
        bool callPython(PyObject * pFunc, avg::EventPtr pEvent);

        EventHandlerMap m_EventHandlerMap;

        CanvasWeakPtr m_pCanvas;

        float m_Opacity;
        NodeState m_State;

        bool m_bActive;
        bool m_bSensitive;
        float m_EffectiveOpacity;
        bool m_bEffectiveActive;
};

}

#endif
