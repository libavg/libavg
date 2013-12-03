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

#include "MultiContextCanvas.h"

#include "Player.h"
#include "SDLDisplayEngine.h"
#include "AVGNode.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/Logger.h"

#include "../graphics/GLContext.h"
#include "../graphics/SecondaryGLXContext.h"

#include <X11/Xlib.h>

#include <vector>

using namespace boost;
using namespace std;

namespace avg {
    
MultiContextCanvas::MultiContextCanvas(Player * pPlayer)
    : MainCanvas(pPlayer)
{
}

MultiContextCanvas::~MultiContextCanvas()
{
}

void MultiContextCanvas::initPlayback(const SDLDisplayEnginePtr& pDisplayEngine)
{
    MainCanvas::initPlayback(pDisplayEngine);
//    createSecondWindow();
}

BitmapPtr MultiContextCanvas::screenshot() const
{
    AVG_ASSERT_MSG(false, "Not implemented");
    return BitmapPtr();
}

static ProfilingZoneID SecondWindowRenderProfilingZone(
        "MultiContextCanvas: render second window");

void MultiContextCanvas::renderTree()
{
    MainCanvas::renderTree();
/*
    {
        GLContext* pMainContext = GLContext::getCurrent();
        ScopeTimer Timer(SecondWindowRenderProfilingZone);
        m_pGLContext->activate();
        Canvas::render(false);
        m_pGLContext->swapBuffers();
        pMainContext->activate();
    }
*/
//    pollEvents();
}

void MultiContextCanvas::createSecondWindow()
{
    GLContext* pMainContext = GLContext::getCurrent();
    const GLConfig& config = pMainContext->getConfig();
    m_pGLContext = SecondaryGLXContextPtr(new SecondaryGLXContext(config, ":0.0",
            IntRect(0,0,800,600)));
    pMainContext->activate();
}

/*
void MultiContextCanvas::pollEvents()
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
