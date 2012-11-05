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

#include "EGLContext.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <EGL/egl.h>

#include <iostream>

namespace avg{

using namespace std;

EGContext::EGContext(const GLConfig& glConfig, const IntPoint& windowSize,
            const SDL_SysWMinfo* pSDLWMInfo):
        GLContext(glConfig, windowSize, pSDLWMInfo)
    {
        createEGContext(glConfig, windowSize, pSDLWMInfo);
        init(true);
    };

EGContext::~EGContext(){
    eglTerminate(m_Display);

}

void EGContext::createEGContext(const GLConfig& glConfig, const IntPoint& windowSize,
        const SDL_SysWMinfo* pSDLWMInfo){

    if (pSDLWMInfo) {
        // SDL window exists, use it.
        m_xDisplay = (EGLNativeDisplayType)pSDLWMInfo->info.x11.display;
    } else {
        m_xDisplay = (EGLNativeDisplayType)XOpenDisplay(0);
    }
    if (!m_xDisplay) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No X windows display available.");
    }

    eglWindow = 0;
    if (pSDLWMInfo) {
        XSetWindowAttributes swa;
        eglWindow = XCreateWindow(m_xDisplay, pSDLWMInfo->info.x11.window, 
                0, 0, windowSize.x, windowSize.y, 0, CopyFromParent, InputOutput, 
                CopyFromParent, CWEventMask, &swa);
        AVG_ASSERT(eglWindow);
        XMapWindow(m_xDisplay, eglWindow);
    }
 

    m_Display = eglGetDisplay(m_xDisplay);
    if ( m_Display == EGL_NO_DISPLAY){
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No EGL display available.");
    }
    if ( !eglInitialize(m_Display, NULL, NULL)){
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Unable to initialize EGL.");
    }

    if( !eglBindAPI(EGL_OPENGL_ES_API)){
        cerr << "Failed to bind GLES API to EGL\n";
        return;
    }

    if ( !eglChooseConfig(m_Display, attribute_list, &m_Config, 1, &m_num_FBConfig)){
        cerr << "Failed to choose config (eglError: " << eglGetError() << ")" << endl;
        return;
    }
    if ( m_num_FBConfig != 1){
        cerr << "Didn't get exactly one config, but " << m_num_FBConfig << endl;
        return;
    }
    if(eglWindow){
        cout << "HAVE WINDOW\n";
        m_Surface = eglCreateWindowSurface(m_Display, m_Config, eglWindow, NULL);
    }else{
        cout << "DON'T have Window\n";
        XVisualInfo visTemplate, *results;
        visTemplate.screen = 0;
        int numVisuals;
        results = XGetVisualInfo(m_xDisplay, VisualScreenMask,
                    &visTemplate, & numVisuals);
        
        Pixmap pmp = XCreatePixmap(m_xDisplay, 
                RootWindow(m_xDisplay, results[0].screen), 8, 8, results[0].depth);
        m_Surface = eglCreatePixmapSurface(m_Display, m_Config, pmp, NULL);
    }
    if ( m_Surface == EGL_NO_SURFACE){
        cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << endl;
        return;
    }
    EGLint pi32ContextAttribs[3];
    pi32ContextAttribs[0] = EGL_CONTEXT_CLIENT_VERSION;
    pi32ContextAttribs[1] = 2;
    pi32ContextAttribs[2] = EGL_NONE;

    m_Context = eglCreateContext(m_Display, m_Config, NULL, pi32ContextAttribs);
    if (m_Context == EGL_NO_CONTEXT ){
        cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << endl;
        return;
    }
    eglMakeCurrent( m_Display, m_Surface, m_Surface, m_Context);
};

bool EGContext::initVBlank(int rate){
    static bool s_bVBlankActive = false;
    return false;
};

void EGContext::activate()
{
    eglMakeCurrent( m_Display, m_Surface, m_Surface, m_Context);
    setCurrent();
};

void EGContext::swapBuffers()
{
    eglSwapBuffers(m_Display, m_Surface);
};

} /* avg */ 
