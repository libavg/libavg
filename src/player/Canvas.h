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

#ifndef _Canvas_H_
#define _Canvas_H_

#include "../api.h"

#include "../base/IPlaybackEndListener.h"
#include "../base/IFrameEndListener.h"
#include "../base/IPreRenderListener.h"
#include "../base/Signal.h"

#include "../graphics/Bitmap.h"

#include <map>
#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

namespace avg {

class Player;
class Node;
class VisibleNode;
class CanvasNode;
class AudioEngine;
class DisplayEngine;
class SDLDisplayEngine;
class TestHelper;
class ProfilingZone;
class Canvas;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::shared_ptr<VisibleNode> VisibleNodePtr;
typedef boost::weak_ptr<VisibleNode> VisibleNodeWeakPtr;
typedef boost::shared_ptr<CanvasNode> CanvasNodePtr;

class AVG_API Canvas: public boost::enable_shared_from_this<Canvas>
{
    public:
        Canvas(Player * pPlayer);
        virtual ~Canvas();
        virtual void setRoot(NodePtr pRootNode);
        virtual void initPlayback(SDLDisplayEngine* pDisplayEngine, 
                AudioEngine* pAudioEngine) = 0;
        void initPlayback(SDLDisplayEngine* pDisplayEngine, AudioEngine* pAudioEngine,
                int multiSampleSamples);
        virtual void stopPlayback();
       
        CanvasNodePtr getRootNode() const;
        VisibleNodePtr getElementByID(const std::string& id);
        void registerNode(VisibleNodePtr pNode);
        void addNodeID(VisibleNodePtr pNode);
        void removeNodeID(const std::string& id);
        virtual void doFrame(bool bPythonAvailable);
        IntPoint getSize() const;
        virtual BitmapPtr screenshot() const = 0;

        void registerPlaybackEndListener(IPlaybackEndListener* pListener);
        void unregisterPlaybackEndListener(IPlaybackEndListener* pListener);
        void registerFrameEndListener(IFrameEndListener* pListener);
        void unregisterFrameEndListener(IFrameEndListener* pListener);
        void registerPreRenderListener(IPreRenderListener* pListener);
        void unregisterPreRenderListener(IPreRenderListener* pListener);

        std::vector<VisibleNodeWeakPtr> getElementsByPos(const DPoint& Pos) const;

        bool operator ==(const Canvas& other) const;
        bool operator !=(const Canvas& other) const;
        long getHash() const;

    protected:
        Player * getPlayer() const;
        SDLDisplayEngine* getDisplayEngine() const;
        void render(IntPoint windowSize, bool bUpsideDown,
                ProfilingZone& renderProfilingZone);

    private:
        virtual void render()=0;
        void renderOutlines();
        Player * m_pPlayer;
        CanvasNodePtr m_pRootNode;
        SDLDisplayEngine * m_pDisplayEngine;
       
        typedef std::map<std::string, VisibleNodePtr> NodeIDMap;
        NodeIDMap m_IDMap;

        Signal<IPlaybackEndListener> m_PlaybackEndSignal;
        Signal<IFrameEndListener> m_FrameEndSignal;
        Signal<IPreRenderListener> m_PreRenderSignal;

        int m_MultiSampleSamples;
};

}
#endif
