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

#include <X11/Xlib.h>

#include <vector>

using namespace boost;
using namespace std;

namespace avg {
    
MultiContextCanvas::MultiContextCanvas(Player * pPlayer)
    : Canvas(pPlayer)
{
}

MultiContextCanvas::~MultiContextCanvas()
{
}

void MultiContextCanvas::setRoot(NodePtr pRootNode)
{
    Canvas::setRoot(pRootNode);
    if (!dynamic_pointer_cast<AVGNode>(pRootNode)) {
        throw (Exception(AVG_ERR_XML_PARSE,
                    "Root node of an avg tree needs to be an <avg> node."));
    }
}

void MultiContextCanvas::initPlayback(const SDLDisplayEnginePtr& pDisplayEngine)
{
    m_pDisplayEngine = pDisplayEngine;
    Canvas::initPlayback(GLContext::getCurrent()->getConfig().m_MultiSampleSamples);

    createSecondWindow();
}

BitmapPtr MultiContextCanvas::screenshot() const
{
    if (!m_pDisplayEngine) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "MultiContextCanvas::screenshot(): Canvas is not being rendered. No screenshot available."));
    }
    return m_pDisplayEngine->screenshot();
}

static ProfilingZoneID RootRenderProfilingZone("Render MultiContextCanvas");

void MultiContextCanvas::renderTree()
{
    preRender();
    glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GLContext::checkError("Canvas::renderTree: BindFramebuffer()");
    {
        ScopeTimer Timer(RootRenderProfilingZone);
        IntPoint windowSize = m_pDisplayEngine->getWindowSize();
        glViewport(0, 0, windowSize.x, windowSize.y);
        Canvas::render(false);
        glXMakeCurrent(m_pDisplay, m_SecondWindow, m_GLContext);
        glEnable(GL_BLEND);
        GLContext::checkError("init: glEnable(GL_BLEND)");
        glDisable(GL_DEPTH_TEST);
        GLContext::checkError("init: glDisable(GL_DEPTH_TEST)");
        glEnable(GL_STENCIL_TEST);
        GLContext::checkError("init: glEnable(GL_STENCIL_TEST)");  
        Canvas::render(false);
        glXSwapBuffers(m_pDisplay, glXGetCurrentDrawable());
        GLContext::getCurrent()->activate();
    }
    pollEvents();
}

void MultiContextCanvas::createSecondWindow()
{
    m_pDisplay = XOpenDisplay((char *)0);
    AVG_ASSERT(m_pDisplay);

    ::Window rootWindow = DefaultRootWindow(m_pDisplay);
    GLint attribs[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    ::XVisualInfo* pVI = glXChooseVisual(m_pDisplay, 0, attribs);
    AVG_ASSERT(pVI);
    ::Colormap cmap = XCreateColormap(m_pDisplay, rootWindow, pVI->visual, AllocNone);
    AVG_ASSERT(cmap);
    XSetWindowAttributes swa;
    swa.event_mask = ButtonPressMask;
    swa.colormap = cmap;

    m_SecondWindow = XCreateWindow(m_pDisplay, rootWindow, 0, 0, 800, 600, 
            5, pVI->depth, InputOutput, pVI->visual, CWColormap | CWEventMask, &swa);
    AVG_ASSERT(m_SecondWindow);
    XMapWindow(m_pDisplay, m_SecondWindow);
    XStoreName(m_pDisplay, m_SecondWindow, "libavg secondary window");
    ::GLXContext otherContext = glXGetCurrentContext();
    m_GLContext = glXCreateContext(m_pDisplay, pVI, otherContext, GL_TRUE);
    AVG_ASSERT(m_GLContext);
//    glXMakeCurrent(dpy, win, ctx);
    
//    XCloseDisplay(m_pDisplay);
}

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

}
