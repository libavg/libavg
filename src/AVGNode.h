//
// $Id$
// 

#ifndef _AVGNode_H_
#define _AVGNode_H_

#include "AVGRegion.h"
#include "IAVGNode.h"
#include "AVGPoint.h"
#include "AVGRect.h"

#include <vector>
#include <string>

#include <paintlib/plstdpch.h>

#include <xpcom/nsCOMPtr.h>
#include <xpcom/nsIComponentManager.h>
#include <jsapi.h>

class AVGContainer;
class AVGEvent;
class AVGRegion;
class IAVGDisplayEngine;
class AVGPlayer;
class AVGMouseEvent;

class AVGNode: public IAVGNode
{
    public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGNODE

        AVGNode ();
        virtual ~AVGNode ();
        void init(const std::string& id, IAVGDisplayEngine * pEngine,
                AVGContainer * pParent, AVGPlayer * pPlayer);
        void initVisible(double x, double y, int z, double width, double height, 
                double opacity, double angle, double pivotx, double pivoty);
        
        virtual void InitEventHandlers
            (const std::string& MouseMoveHandler, 
             const std::string& MouseButtonUpHandler, 
             const std::string& MouseButtonDownHandler,
             const std::string& MouseOverHandler, 
             const std::string& MouseOutHandler);

        virtual AVGNode * getElementByPos (const AVGDPoint & pos);
        virtual void prepareRender (int time, const AVGDRect& parent);
        virtual void maybeRender (const AVGDRect& Rect);
        virtual void render (const AVGDRect& Rect);
        virtual bool obscures (const AVGDRect& Rect, int z);
        virtual void addDirtyRect(const AVGDRect& Rect);
        virtual void getDirtyRegion (AVGRegion& Region);
        virtual void setViewport (double x, double y, double width, double height);
        virtual const AVGDRect& getRelViewport ();
        virtual const AVGDRect& getAbsViewport();
        AVGDRect AVGNode::getVisibleRect();
        virtual int getZ();
        double getOpacity();
        double getAngle();
        void setOpacity(double o);
        virtual double getEffectiveOpacity();

        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
        virtual const std::string& getID ();
        virtual AVGContainer * getParent();
        
        virtual bool handleMouseEvent (AVGMouseEvent* pEvent, 
                JSContext * pJSContext);

    protected:
        virtual void invalidate();
        virtual AVGDPoint getPreferredMediaSize() = 0;
        AVGDPoint getPivot();
        AVGPlayer * getPlayer();
        IAVGDisplayEngine * getEngine();

    private:
        void callJS (const std::string& Code, JSContext * pJSContext);
        void calcAbsViewport();

        std::string m_ID;
        AVGContainer * m_pParent;
        IAVGDisplayEngine * m_pEngine;
        AVGPlayer * m_pPlayer;

        std::string m_MouseMoveHandler;
        std::string m_MouseButtonUpHandler;
        std::string m_MouseButtonDownHandler;
        std::string m_MouseOverHandler;
        std::string m_MouseOutHandler;

        AVGDRect m_RelViewport;      // In coordinates relative to the parent.
        AVGDRect m_AbsViewport;      // In window coordinates.
        int m_z;
        double m_Opacity;
        double m_Angle;
        bool m_bHasCustomPivot;
        AVGDPoint m_Pivot;
        
        AVGRegion m_DirtyRegion;

};

template<class NODECLASS>
NODECLASS* createNode(std::string CID) {
    nsresult rv;
    IAVGNode* pINode;
    rv = nsComponentManager::CreateInstance (CID.c_str(), 0,
            NS_GET_IID(IAVGNode), (void**)&pINode);
    if (NS_FAILED(rv)) {
        std::cerr << "createNode failed: " << rv << std::endl;
        PLASSERT(false);
    }
    NODECLASS * pNode = dynamic_cast<NODECLASS*>((IAVGNode*)pINode);
    return pNode;
}

#endif //_AVGNode_H_

