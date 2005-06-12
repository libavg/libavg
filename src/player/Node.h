//
// $Id$
// 

#ifndef _Node_H_
#define _Node_H_

#include "../Object.h"

#include "Region.h"
#include "Point.h"
#include "Rect.h"
#include "ISurface.h"

#include <vector>
#include <string>

#include "../js/jsapi.h"

namespace avg {

class Container;
class Event;
class Region;
class IDisplayEngine;
class Player;
class MouseEvent;
class OGLSurface;

class Node: public Object
{
    public:
        Node ();
        virtual ~Node ();
        virtual void init(IDisplayEngine * pEngine, Container * pParent,
                Player * pPlayer);
        virtual void initVisible();
        
        // JS interface
        void setZ(int z);
        void setActive(bool bActive);

        /**
         * Returns the unique id that can be used to reference the node.
         */
        virtual const std::string& getID ();
        /**
         * Returns the parent node, if there is one.
         */
        virtual Container * getParent();

        bool isActive();
        virtual Node * getElementByPos (const DPoint & pos);
        virtual void prepareRender (int time, const DRect& parent);
        virtual void maybeRender (const DRect& Rect);
        virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int z);
        virtual void addDirtyRect(const DRect& Rect);
        virtual void getDirtyRegion (Region& Region);
        virtual void setViewport (double x, double y, double width, 
                double height);
        virtual const DRect& getRelViewport ();
        virtual const DRect& getAbsViewport();
        DRect getVisibleRect();
        virtual int getZ();
        double getOpacity();
        void setOpacity(double o);
        virtual double getEffectiveOpacity();

        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
        void setParent(Container * pParent);
        
        virtual void handleMouseEvent (MouseEvent* pEvent, 
                JSContext * pJSContext);
        virtual void invalidate();
        
        // TODO: Do we still need this? Isn't rtti good enough?
        enum {NT_UNKNOWN, NT_IMAGE, NT_AVG, NT_VIDEO, NT_TEXT, NT_EXCL, 
                NT_CAMERA, NT_DIV, NT_PANOIMAGE};

    protected:
        virtual DPoint getPreferredMediaSize() 
            { return DPoint(0,0); };
        Player * getPlayer();
        IDisplayEngine * getEngine();

//        void callJS (const std::string& Code, JSContext * pJSContext);
        void initFilename(Player * pPlayer, std::string& sFilename);
        bool isInitialized();
 
    private:
        void calcAbsViewport();

        Container * m_pParent;
        IDisplayEngine * m_pEngine;
        Player * m_pPlayer;

        std::string m_ID;
        std::string m_MouseMoveHandler;
        std::string m_MouseButtonUpHandler;
        std::string m_MouseButtonDownHandler;
        std::string m_MouseOverHandler;
        std::string m_MouseOutHandler;

        DRect m_RelViewport;      // In coordinates relative to the parent.
        DRect m_AbsViewport;      // In window coordinates.
        int m_z;
        double m_Opacity;
        bool m_bActive;
        
        // Initialization helpers.
        bool m_bInitialized;
        int m_InitialWidth;
        int m_InitialHeight;

        Region m_DirtyRegion;
};

}

#endif //_Node_H_

