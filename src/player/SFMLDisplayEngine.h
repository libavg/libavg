#ifndef _SFMLDisplayEngine_H_
#define _SFMLDisplayEngine_H_

#include "../api.h"
#include "IInputDevice.h"
#include "DisplayEngine.h"

#include "../graphics/GLConfig.h"
#include "../graphics/Bitmap.h"
#include "../graphics/Pixel32.h"
#include "../graphics/OGLHelper.h"
#include "../graphics/FBO.h"

#include <string>
#include <vector>

#undef None     //Why is none defined?
#include <SFML/Window.hpp>

typedef sf::Event SFML_Event;

namespace avg {

class Input;
class Window;

class XInputMTInputDevice;
class MouseEvent;
typedef boost::shared_ptr<class MouseEvent> MouseEventPtr;
class GLContext;

class AVG_API SFMLDisplayEngine: public DisplayEngine, public IInputDevice
{
    public:
        SFMLDisplayEngine();
        virtual ~SFMLDisplayEngine();
        virtual void init(const DisplayParams& dp, GLConfig glConfig);

        // From DisplayEngine
        virtual void teardown();
        virtual float getRefreshRate();
        virtual void setGamma(float red, float green, float blue);
        virtual void setMousePos(const IntPoint& pos);
        virtual int getKeyModifierState() const;

        virtual IntPoint getSize();

        virtual void showCursor(bool bShow);
        virtual BitmapPtr screenshot(int buffer=0);

        // From IInputDevice
        virtual std::vector<EventPtr> pollEvents();
        void setXIMTInputDevice(XInputMTInputDevice* pInputDevice);

        const IntPoint& getWindowSize() const;
        bool isFullscreen() const;
        IntPoint getScreenResolution();
        float getPixelsPerMM();
        glm::vec2 getPhysicalScreenDimensions();
        void assumePixelsPerMM(float ppmm);
        virtual void swapBuffers();

    private:
        void initSFML(int width, int height, bool isFullscreen, int bpp);
        void initTranslationTable();
        void calcScreenDimensions(float dotsPerMM=0);

        bool internalSetGamma(float red, float green, float blue);

        EventPtr createMouseEvent
                (Event::Type Type, const SFML_Event & sfmlEvent, long Button);
        EventPtr createMouseButtonEvent(Event::Type Type, const SFML_Event & sfmlEvent);
        EventPtr createMouseWheelEvent(const SFML_Event & sfmlEvent);
        void createKeyEvent(Event::Type Type, const SFML_Event & sfmlEvent);

        IntPoint m_Size;
        bool m_bIsFullscreen;
        IntPoint m_WindowSize;
        IntPoint m_ScreenResolution;
        float m_PPMM;

        const sf::Input* m_input;
        sf::Window* m_pScreen;

        static void calcRefreshRate();
        static float s_RefreshRate;

        // Event handling.
        bool m_bMouseOverApp;
        MouseEventPtr m_pLastMouseEvent;
        int m_NumMouseButtonsDown;
        static std::vector<long> KeyCodeTranslationTable;
        XInputMTInputDevice* m_pXIMTInputDevice;

        GLContext* m_pGLContext;

        float m_Gamma[3];
};

typedef boost::shared_ptr<SFMLDisplayEngine> SFMLDisplayEnginePtr;

}

#endif
