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

#ifndef _OffscreenCanvas_H_
#define _OffscreenCanvas_H_

#include "../api.h"
#include "Canvas.h"
#include "OffscreenCanvasNode.h"

#include "CameraNode.h"

#include "../graphics/FBO.h"

#include <string>

namespace avg {

class AVG_API OffscreenCanvas: public Canvas
{
    public:
        OffscreenCanvas(Player * pPlayer);
        virtual ~OffscreenCanvas();
        virtual void setRoot(NodePtr pRootNode);
        virtual void initPlayback();
        virtual void stopPlayback(bool bIsAbort);

        virtual BitmapPtr screenshot() const;
        bool getHandleEvents() const;
        int getMultiSampleSamples() const;
        bool getMipmap() const;
        bool getAutoRender() const;
        void setAutoRender(bool bAutoRender);
        void manualRender(); // This is the render that can be called from python.

        std::string getID() const;
        bool isRunning() const;
        GLTexturePtr getTex() const;
        FBOPtr getFBO() const;

        void registerCameraNode(CameraNode* pCameraNode);
        void unregisterCameraNode();
        void updateCameraImage();
        bool hasRegisteredCamera() const;
        bool isCameraImageAvailable() const;

        void addDependentCanvas(CanvasPtr pCanvas);
        void removeDependentCanvas(CanvasPtr pCanvas);
        const std::vector<CanvasPtr>& getDependentCanvases() const;
        unsigned getNumDependentCanvases() const;

        static bool isSupported();
        static bool isMultisampleSupported();
        void dump() const;
 
    protected:
        virtual void renderTree();

    private:
        FBOPtr m_pFBO;
        bool m_bUseMipmaps;
        std::vector<CanvasPtr> m_pDependentCanvases;

        bool m_bIsRendered;
        CameraNode* m_pCameraNodeRef;
};

typedef boost::shared_ptr<OffscreenCanvas> OffscreenCanvasPtr;

}
#endif

