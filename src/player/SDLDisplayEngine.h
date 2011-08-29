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

#ifndef _SDLDisplayEngine_H_
#define _SDLDisplayEngine_H_

#include "../api.h"
#include "IInputDevice.h"
#include "DisplayEngine.h"
#include "GLConfig.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Pixel32.h"
#include "../graphics/OGLHelper.h"
#include "../graphics/FBO.h"

#include <string>
#include <vector>

struct SDL_Surface;
union SDL_Event;

namespace avg {

class XInput21MTInputDevice;
class MouseEvent;
typedef boost::shared_ptr<class MouseEvent> MouseEventPtr;

class AVG_API SDLDisplayEngine: public DisplayEngine, public IInputDevice
{
    public:
        SDLDisplayEngine();
        virtual ~SDLDisplayEngine();

        // From DisplayEngine
        virtual void init(const DisplayParams& dp);
        virtual void teardown();
        virtual double getRefreshRate();
        virtual void setGamma(double red, double green, double blue);
        virtual void setMousePos(const IntPoint& pos);
        virtual int getKeyModifierState() const;

        virtual void pushClipRect(VertexArrayPtr pVA);
        virtual void popClipRect(VertexArrayPtr pVA);
        virtual void pushTransform(const DPoint& translate, double angle, 
                const DPoint& pivot);
        virtual void popTransform();

        virtual IntPoint getSize();

        virtual bool isUsingShaders() const; 
        
        virtual void showCursor(bool bShow);
        virtual BitmapPtr screenshot();

        // From IInputDevice
        virtual std::vector<EventPtr> pollEvents();
        void setXIMTInputDevice(XInput21MTInputDevice* pInputDevice);

        // Texture config.
        void initTextureMode();
        bool usePOTTextures();
        int getMaxTexSize();

        // OpenGL state setting.
        void enableTexture(bool bEnable);
        void enableGLColorArray(bool bEnable);
        void setBlendMode(BlendMode mode, bool bPremultipliedAlpha = false);
        
        OGLMemoryMode getMemoryModeSupported();

        void setOGLOptions(const GLConfig& glConfig);
        const GLConfig& getOGLOptions() const;
        const IntPoint& getWindowSize() const;
        bool isFullscreen() const;
        IntPoint getScreenResolution();
        double getPixelsPerMM();
        DPoint getPhysicalScreenDimensions();
        void assumePhysicalScreenDimensions(const DPoint& size);

    private:
        void initSDL(int width, int height, bool isFullscreen, int bpp);
        void initTranslationTable();
        void logConfig();
        void calcScreenDimensions(const DPoint& physScreenSize=DPoint(0,0));
        virtual void swapBuffers();
        void clip(VertexArrayPtr pVA, GLenum stencilOp);

        EventPtr createMouseEvent
                (Event::Type Type, const SDL_Event & SDLEvent, long Button);
        EventPtr createMouseButtonEvent
                (Event::Type Type, const SDL_Event & SDLEvent);
        EventPtr createKeyEvent
                (Event::Type Type, const SDL_Event & SDLEvent);
        
        IntPoint m_Size;
        bool m_bIsFullscreen;
        IntPoint m_WindowSize;
        IntPoint m_ScreenResolution;
        DPoint m_PPMM;
        int m_ClipLevel;

        SDL_Surface * m_pScreen;

        void checkShaderSupport();

        // Vertical blank stuff.
        virtual bool initVBlank(int rate);
        void initMacVBlank(int rate);
        bool vbWait(int rate);
        enum VBMethod {VB_SGI, VB_APPLE, VB_WIN, VB_NONE};
        VBMethod m_VBMethod;
        int m_VBMod;
        int m_LastVBCount;
        bool m_bFirstVBFrame;

        static void calcRefreshRate();
        static double s_RefreshRate;

        // Event handling.
        bool m_bMouseOverApp;
        MouseEventPtr m_pLastMouseEvent;
        int m_NumMouseButtonsDown;
        static std::vector<long> KeyCodeTranslationTable;
        XInput21MTInputDevice * m_pXIMTInputDevice;

        int m_MaxTexSize;

        // OpenGL state
        bool m_bEnableTexture;
        bool m_bEnableGLColorArray;
        BlendMode m_BlendMode;
        bool m_bPremultipliedAlpha;

        GLConfig m_GLConfig;
        
        bool m_bCheckedMemoryMode;
        OGLMemoryMode m_MemoryMode;

        FBOPtr m_pFBO;

        double m_Gamma[3];
};

}

#endif
