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
#include "DisplayEngine.h"
#include "AVGNode.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/Logger.h"

#include "../graphics/GLContext.h"
#include "../graphics/GLTexture.h"
#include "../graphics/SecondaryGLXContext.h"
#include "../graphics/GLContextMultiplexer.h"

#include <X11/Xlib.h>

#include <vector>

using namespace boost;
using namespace std;

namespace avg {

MainCanvas::MainCanvas(Player * pPlayer, bool bSecondViewport)
    : Canvas(pPlayer),
      m_bSecondViewport(bSecondViewport)
{
    m_pMultiplexer = GLContextMultiplexerPtr(new GLContextMultiplexer());
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

    if (m_bSecondViewport) {
        createSecondWindow();
    }
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
static ProfilingZoneID SecondWindowRenderProfilingZone(
        "Render second window");

void MainCanvas::renderTree()
{
    preRender();
    m_pMultiplexer->uploadData();
    glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GLContext::checkError("Canvas::renderTree: BindFramebuffer()");
    {
        ScopeTimer Timer(RootRenderProfilingZone);
        IntPoint windowSize = m_pDisplayEngine->getWindowSize();
        glViewport(0, 0, windowSize.x, windowSize.y);
        Canvas::render(false);
    }

    if (m_bSecondViewport) {
        GLContext* pMainContext = GLContext::getCurrent();
        ScopeTimer Timer(SecondWindowRenderProfilingZone);
        m_pGLContext->activate();
        m_pMultiplexer->uploadData();
        Canvas::render(false);
        m_pGLContext->swapBuffers();
        pMainContext->activate();
    }

    m_pMultiplexer->reset();
//    pollEvents();
}

void MainCanvas::createSecondWindow()
{
    GLContext* pMainContext = GLContext::getCurrent();
    const GLConfig& config = pMainContext->getConfig();
    m_pGLContext = SecondaryGLXContextPtr(new SecondaryGLXContext(config, ":0.0",
            IntRect(0,0,800,600)));
    
    pMainContext->activate();
}

/*
void MainCanvas::pollEvents()
{
    XEvent xev;
    while (XCheckWindowEvent(m_pDisplay, m_SecondWindow, ButtonPressMask, &xev)) {
        switch(xev.type) {
            case ButtonPress:
                cerr << "..." << endl;
                break;
            default:
                cerr << "?" << endl;
        }
    }
}
*/

}
