//
// $Id$
//

#include "AVGEvent.h"

#include <paintlib/pldebug.h>

#include <iostream.h>

AVGEvent::AVGEvent()
    : m_Type(0)
{
    NS_INIT_ISUPPORTS();
}

AVGEvent::~AVGEvent()
{
}

void AVGEvent::init(int type, const PLPoint& pos, int buttonState, int keySym)
{
    m_Type = type;
    m_Pos = pos;
    m_ButtonState = buttonState;
    m_KeySym = keySym;
    m_KeyMod = 0;
}

void AVGEvent::init(const SDL_Event& SDLEvent)
{
    switch(SDLEvent.type){
        case SDL_MOUSEMOTION:
            m_Type=MOUSEMOVE;
            m_Pos=PLPoint(SDLEvent.motion.x, SDLEvent.motion.y);
            m_ButtonState=SDLEvent.motion.state;
            break;
        case SDL_MOUSEBUTTONDOWN:
            m_Type=MOUSEDOWN;
            m_Pos = PLPoint(SDLEvent.button.x, SDLEvent.button.y);
            m_ButtonState=SDLEvent.button.state;
            break;
        case SDL_MOUSEBUTTONUP:
            m_Type=MOUSEUP;
            m_Pos = PLPoint(SDLEvent.button.x, SDLEvent.button.y);
            m_ButtonState=SDLEvent.button.state;
            break;
        case SDL_KEYDOWN:
            m_Type=KEYDOWN;
            m_KeySym=SDLEvent.key.keysym.sym;
            m_KeyMod=SDLEvent.key.keysym.mod;
            break;
        case SDL_KEYUP:
            m_Type=KEYUP;
            m_KeySym=SDLEvent.key.keysym.sym;
            m_KeyMod=SDLEvent.key.keysym.mod;
            break;
        case SDL_QUIT:
            m_Type=QUIT;
            break;
        default:
            cerr << "AVGEvent::init(): Illegal event type" << endl;
            break;
    }
}

NS_IMPL_ISUPPORTS1(AVGEvent, IAVGEvent);

NS_IMETHODIMP AVGEvent::GetType(PRInt32 *_retval)
{
    PLASSERT (m_Type != 0);
    *_retval = m_Type;
    return NS_OK;
}

NS_IMETHODIMP AVGEvent::GetXPos(PRInt32 *_retval)
{
    PLASSERT(m_Type != KEYDOWN && m_Type != KEYUP);
    *_retval = m_Pos.x;
    return NS_OK;
}

NS_IMETHODIMP AVGEvent::GetYPos(PRInt32 *_retval)
{
    PLASSERT(m_Type != KEYDOWN && m_Type != KEYUP);
    *_retval = m_Pos.y;
    return NS_OK;
}

NS_IMETHODIMP AVGEvent::GetMouseButtonState(PRInt32 *_retval)
{
    PLASSERT(m_Type != KEYDOWN && m_Type != KEYUP);
    *_retval = m_ButtonState;
    return NS_OK;
}

NS_IMETHODIMP AVGEvent::GetKeySym(PRInt32 *_retval)
{
    PLASSERT(m_Type == KEYDOWN || m_Type == KEYUP);
    *_retval = m_KeySym;
    return NS_OK;
}

NS_IMETHODIMP AVGEvent::GetKeyMod(PRInt32 *_retval)
{
    PLASSERT(m_Type == KEYDOWN || m_Type == KEYUP);
    *_retval = m_KeyMod;
    return NS_OK;
}

