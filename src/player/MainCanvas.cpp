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

#include "MainCanvas.h"

#include "Player.h"
#include "DisplayEngine.h"
#include "AVGNode.h"
#include "Window.h"
#include "RenderThread.h"
#include "SecondaryWindow.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/Logger.h"

#include "../graphics/GLContext.h"
#include "../graphics/GLTexture.h"
#include "../graphics/GLContextManager.h"
#ifdef __linux__
  #ifndef AVG_ENABLE_EGL
  #include "../graphics/SecondaryGLXContext.h"
  #include <X11/Xlib.h>
  #endif
#endif

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

void MainCanvas::initPlayback(const DisplayEnginePtr& pDisplayEngine)
{
    m_pDisplayEngine = pDisplayEngine;
    Canvas::initPlayback(GLContext::getCurrent()->getConfig().m_MultiSampleSamples);
}

BitmapPtr MainCanvas::screenshot() const
{
    if (!m_pDisplayEngine) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "MainCanvas::screenshot(): Canvas is not being rendered. No screenshot available."));
    }
    return m_pDisplayEngine->screenshot();
}

void MainCanvas::onRenderDone()
{
    boost::mutex::scoped_lock lock(m_RenderMutex);
    m_NumThreadsRunning--;
    m_RenderCondition.notify_one();
}

static ProfilingZoneID RootRenderProfilingZone("Render MainCanvas");

void MainCanvas::renderTree()
{
    preRender();
    unsigned numWindows = m_pDisplayEngine->getNumWindows();
    m_NumThreadsRunning = numWindows-1;
    ScopeTimer Timer(RootRenderProfilingZone);
    GLContextManager* pCM = GLContextManager::get();
    pCM->uploadData();
    pCM->setRenderPhase(true);

    for (unsigned i=1; i<numWindows; ++i) {
        WindowPtr pWindow = m_pDisplayEngine->getWindow(i);
        IntRect viewport = pWindow->getViewport();
        dynamic_pointer_cast<SecondaryWindow>(pWindow)->
                render(this, viewport, MCFBOPtr());
    }
    WindowPtr pWindow = m_pDisplayEngine->getWindow(0);
    IntRect viewport = pWindow->getViewport();
    renderWindow(&*pWindow, MCFBOPtr(), viewport);

    if (numWindows > 1) {
        boost::mutex::scoped_lock lock(m_RenderMutex);
        while (m_NumThreadsRunning) {
            m_RenderCondition.wait(lock);
        }
    }
    GLContextManager::get()->setRenderPhase(false);
    GLContextManager::get()->reset();
}

}
