//
// $Id$
// 

#ifndef _AVGNode_H_
#define _AVGNode_H_

#include "AVGRegion.h"
#include <IAVGNode.h>

#include <vector>
#include <string>

#include <paintlib/plstdpch.h>

#include <xpcom/nsCOMPtr.h>
#include <xpcom/nsIComponentManager.h>
#include <jsapi.h>

class PLPoint;
class PLRect;
class AVGContainer;
class AVGEvent;
class AVGRegion;
class AVGDFBDisplayEngine;
class AVGPlayer;

class AVGNode: public IAVGNode
{
	public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGNODE

        AVGNode ();
        virtual ~AVGNode ();
        void init(const std::string& id, AVGDFBDisplayEngine * pEngine,
                AVGContainer * pParent, AVGPlayer * pPlayer);
        void initVisible(int x, int y, int z, int width, int height, double opacity);
        
        virtual void InitEventHandlers
            (const std::string& MouseMoveHandler, 
             const std::string& MouseButtonUpHandler, 
             const std::string& MouseButtonDownHandler,
             const std::string& MouseOverHandler, 
             const std::string& MouseOutHandler);

        virtual AVGNode * getElementByPos (const PLPoint & pos);
		virtual void prepareRender (int time, const PLRect& parent);
        virtual void maybeRender (const PLRect& Rect);
		virtual void render (const PLRect& Rect);
        virtual bool obscures (const PLRect& Rect, int z);
        virtual void addDirtyRect(const PLRect& Rect);
		virtual void getDirtyRegion (AVGRegion& Region);
        virtual void setViewport (int x, int y, int width, int height);
        virtual const PLRect& getRelViewport ();
        virtual const PLRect& getAbsViewport();
        virtual int getZ();
        double getOpacity();
        void setOpacity(double o);
        virtual double getEffectiveOpacity();
        AVGDFBDisplayEngine * getEngine();

        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
        virtual const std::string& getID ();
        virtual AVGContainer * getParent();
        
        virtual bool handleEvent (AVGEvent* pEvent, JSContext * pJSContext);

    protected:
        virtual void invalidate();
        virtual bool isVisibleNode();  // Poor man's RTTI
        virtual PLPoint getPreferredMediaSize() = 0;
        AVGPlayer * getPlayer();

	private:
        void callJS (const std::string& Code, JSContext * pJSContext);

        std::string m_ID;
		AVGContainer * m_pParent;
        AVGDFBDisplayEngine * m_pEngine;
        AVGPlayer * m_pPlayer;

        std::string m_MouseMoveHandler;
        std::string m_MouseButtonUpHandler;
        std::string m_MouseButtonDownHandler;
        std::string m_MouseOverHandler;
        std::string m_MouseOutHandler;

        double m_Opacity;
        PLRect m_RelViewport;      // In coordinates relative to the parent.
        PLRect m_AbsViewport;      // In window coordinates.
        int m_z;
        
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

