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

#ifndef _Scene_H_
#define _Scene_H_

#include "../api.h"
#include "IEventSink.h"
#include "EventDispatcher.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "CursorState.h"
#include "MouseState.h"

#include "../base/IPlaybackEndListener.h"
#include "../base/IFrameEndListener.h"
#include "../base/IPreRenderListener.h"
#include "../base/Signal.h"

#include "../graphics/Bitmap.h"

#include <map>
#include <string>
#include <vector>

namespace avg {

class Player;
class Node;
class SceneNode;
class SDLDisplayEngine;
class TestHelper;

typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
typedef boost::shared_ptr<SceneNode> SceneNodePtr;

class AVG_API Scene
{
    public:
        Scene(Player * pPlayer, NodePtr pRootNode);
        virtual ~Scene();
        virtual void initPlayback(DisplayEngine* pDisplayEngine, 
                AudioEngine* pAudioEngine, TestHelper* pTestHelper);
        virtual void stopPlayback();
       
        void setEventCapture(NodePtr pNode, int cursorID);
        void releaseEventCapture(int cursorID);

        SceneNodePtr getRootNode() const;
        NodePtr getElementByID(const std::string& id);
        void registerNode(NodePtr pNode);
        void addNodeID(NodePtr pNode);
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

        virtual void render() = 0;
        
        bool operator ==(const Scene& other) const;
        bool operator !=(const Scene& other) const;
        long getHash() const;

    protected:
        Player * getPlayer() const;
        SDLDisplayEngine* getDisplayEngine() const;
        std::map<int, NodeWeakPtr> m_pEventCaptureNode;
        std::vector<NodeWeakPtr> getElementsByPos(const DPoint& Pos) const;

    private:
        Player * m_pPlayer;
        SceneNodePtr m_pRootNode;
        DisplayEngine * m_pDisplayEngine;
       
        typedef std::map<std::string, NodePtr> NodeIDMap;
        NodeIDMap m_IDMap;

        Signal<IPlaybackEndListener> m_PlaybackEndSignal;
        Signal<IFrameEndListener> m_FrameEndSignal;
        Signal<IPreRenderListener> m_PreRenderSignal;
};

}
#endif
