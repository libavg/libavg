//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _Canvas_H_
#define _Canvas_H_
 
#include "../api.h"

#include "FontStyle.h"

#include "../base/IPlaybackEndListener.h"
#include "../base/IFrameEndListener.h"
#include "../base/IPreRenderListener.h"
#include "../base/Signal.h"
#include "../base/GLMHelper.h"

#include "../graphics/OGLHelper.h"
#include "../graphics/Bitmap.h"

#include <map>
#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

namespace avg {

class Player;
class Node;
class RasterNode;
class CanvasNode;
class AudioEngine;
class TestHelper;
class ProfilingZoneID;
class Canvas;
class FBO;
class MCFBO;
class VertexArray;
class SubVertexArray;
class Window;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::shared_ptr<RasterNode> RasterNodePtr;
typedef boost::shared_ptr<CanvasNode> CanvasNodePtr;
typedef boost::shared_ptr<FBO> FBOPtr;
typedef boost::shared_ptr<MCFBO> MCFBOPtr;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;
typedef boost::shared_ptr<Window> WindowPtr;

class Canvas;
typedef boost::shared_ptr<Canvas> CanvasPtr;
typedef boost::weak_ptr<Canvas> CanvasWeakPtr;

class AVG_API Canvas: public ExportedObject
{
    public:
        Canvas(Player * pPlayer);
        virtual ~Canvas();
        virtual void setRoot(NodePtr pRootNode);
        void initPlayback(int multiSampleSamples);
        virtual void stopPlayback(bool bIsAbort);
       
        CanvasNodePtr getRootNode() const;
        NodePtr getElementByID(const std::string& id);
        void registerNode(NodePtr pNode);
        void addNodeID(NodePtr pNode);
        void removeNodeID(const std::string& id);
        virtual void doFrame(bool bPythonAvailable);
        IntPoint getSize() const;
        virtual BitmapPtr screenshot() const = 0;
        virtual void pushClipRect(const glm::mat4& transform, SubVertexArray& va);
        virtual void popClipRect(const glm::mat4& transform, SubVertexArray& va);

        void registerPlaybackEndListener(IPlaybackEndListener* pListener);
        void unregisterPlaybackEndListener(IPlaybackEndListener* pListener);
        void registerFrameEndListener(IFrameEndListener* pListener);
        void unregisterFrameEndListener(IFrameEndListener* pListener);
        void registerPreRenderListener(IPreRenderListener* pListener);
        void unregisterPreRenderListener(IPreRenderListener* pListener);

        std::vector<NodePtr> getElementsByPos(const glm::vec2& Pos) const;

        virtual void renderWindow(WindowPtr pWindow, MCFBOPtr pFBO, 
                const IntRect& viewport);
        void scheduleFXRender(const RasterNodePtr& pNode);

    protected:
        Player * getPlayer() const;
        void preRender();
        void emitPreRenderSignal(); 
        void emitFrameEndSignal();

    private:
        virtual void renderTree()=0;
        void renderFX();
        void resetFXSchedule();
        void renderOutlines(const glm::mat4& transform);

        void clip(const glm::mat4& transform, SubVertexArray& va, GLenum stencilOp);
        Player * m_pPlayer;
        CanvasNodePtr m_pRootNode;
        bool m_bIsPlaying;
        VertexArrayPtr m_pVertexArray;
       
        typedef std::map<std::string, NodePtr> NodeIDMap;
        NodeIDMap m_IDMap;

        Signal<IPlaybackEndListener> m_PlaybackEndSignal;
        Signal<IFrameEndListener> m_FrameEndSignal;
        Signal<IPreRenderListener> m_PreRenderSignal;

        int m_MultiSampleSamples;
        int m_ClipLevel;

        std::vector<RasterNodePtr> m_pScheduledFXNodes;
};

}
#endif
