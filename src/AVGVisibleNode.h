//
// $Id$
//

#ifndef _AVGVisibleNode_H_
#define _AVGVisibleNode_H_

#include "AVGNode.h"
#include "AVGRegion.h"

#include <paintlib/plrect.h>

#include <string>

class PLBmp;
class PLPoint;

class AVGVisibleNode : 	
	public virtual AVGNode
{
    public:
        AVGVisibleNode ();
        virtual ~AVGVisibleNode();

        NS_IMETHOD GetIntAttr(const char *name, PRInt32 *_retval); 
        NS_IMETHOD GetFloatAttr(const char *name, float *_retval); 
        NS_IMETHOD SetIntAttr(const char *name, PRInt32 value); 
        NS_IMETHOD SetFloatAttr(const char *name, float value);

        void init (const std::string& id, int x, int y, int z, 
                int width, int height, double opacity, 
                AVGDFBDisplayEngine * pEngine, AVGContainer * pParent);

        virtual AVGNode * getElementByPos (const PLPoint & pos);
        virtual void update (int time, const PLRect& parent);
        virtual std::string dump (int indent = 0);

        virtual void setViewport (int x, int y, int width, int height);
        const PLRect& getRelViewport ();
        const PLRect& getAbsViewport ();
        virtual int getZ ();
        double getOpacity();
        void setOpacity(double o);
        virtual double getEffectiveOpacity();

    protected:
        void invalidate();

    private:
        double m_Opacity;
        PLRect m_RelViewport;      // In coordinates relative to the parent.
        PLRect m_AbsViewport;      // In window coordinates.
        int m_z;

};

#endif //_AVGVisibleNode_H_

