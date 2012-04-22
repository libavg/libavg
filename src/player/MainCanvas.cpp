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

#include "MainCanvas.h"

#include "Player.h"
#include "SDLDisplayEngine.h"
#include "AVGNode.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/Logger.h"

#include "../graphics/GLContext.h"

#include <vector>

using namespace boost;
using namespace std;

namespace avg {
    
MainCanvas::MainCanvas(Player * pPlayer)
    : Canvas(pPlayer)
{
}

MainCanvas::~MainCanvas()
{
}

void MainCanvas::setRoot(NodePtr pRootNode)
{
    Canvas::setRoot(pRootNode);
    if (!dynamic_pointer_cast<AVGNode>(pRootNode)) {
        throw (Exception(AVG_ERR_XML_PARSE,
                    "Root node of an avg tree needs to be an <avg> node."));
    }
}

void MainCanvas::initPlayback(const SDLDisplayEnginePtr& pDisplayEngine)
{
    m_pDisplayEngine = pDisplayEngine;
    Canvas::initPlayback(GLContext::getMain()->getConfig().m_MultiSampleSamples);
}

BitmapPtr MainCanvas::screenshot() const
{
    if (!m_pDisplayEngine) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "MainCanvas::screenshot(): Canvas is not being rendered. No screenshot available."));
    }
    return m_pDisplayEngine->screenshot();
}

static ProfilingZoneID RootRenderProfilingZone("Render MainCanvas");

void MainCanvas::render()
{
    Canvas::render(m_pDisplayEngine->getWindowSize(), false, FBOPtr(),
            RootRenderProfilingZone);
}

}
