//
// $Id$
//

#include "AVGEvent.h"

#include <paintlib/pldebug.h>

#include "nsMemory.h"

#include <iostream.h>
#include <string>

using namespace std;

AVGEvent::AVGEvent()
    : m_pNode(0),
      m_Type(0)
{
    NS_INIT_ISUPPORTS();
}

AVGEvent::~AVGEvent()
{
}

void AVGEvent::init(int type, const PLPoint& pos, 
        int buttonState, int keySym)
{
    m_Type = type;
    m_Pos = pos;
    m_ButtonState = buttonState;
    m_KeySym = keySym;
    m_KeyMod = 0;
}

void AVGEvent::init(const SDL_Event& SDLEvent)
{
    switch(SDLEvent.type) {
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

void AVGEvent::setNode(IAVGNode* pNode)
{
    m_pNode = pNode;
}

NS_IMPL_ISUPPORTS1_CI(AVGEvent, IAVGEvent);

NS_IMETHODIMP AVGEvent::IsMouseEvent(PRBool *_retval)
{
    *_retval = (m_Type == MOUSEDOWN || m_Type == MOUSEUP || m_Type == MOUSEMOVE ||
                m_Type == MOUSEOVER || m_Type == MOUSEOUT);
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP AVGEvent::GetElement(IAVGNode **_retval)
{
    PLASSERT(m_pNode);
    NS_IF_ADDREF(m_pNode);
    *_retval = m_pNode;
    return NS_OK;
}

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

void AVGEvent::dump(int DebugLevel)
{
    string EventName;
    switch(m_Type) {
        case AVGEvent::MOUSEMOVE: // Mousemotion events aren't dumped.
            return;
        case AVGEvent::MOUSEDOWN:
            EventName =  "MOUSEDOWN";
            break;
        case AVGEvent::MOUSEUP:
            EventName =  "MOUSEUP";
            break;
        case AVGEvent::MOUSEOVER:
            EventName =  "MOUSEOVER";
            break;
        case AVGEvent::MOUSEOUT:
            EventName =  "MOUSEOUT";
            break;
        case AVGEvent::KEYDOWN:
            EventName = "KEYDOWN";
            break;
        case AVGEvent::KEYUP:
            EventName = "KEYUP";
            break;
        case AVGEvent::QUIT:
            EventName = "QUIT";
            break;
        default: 
            cerr << "Illegal event type " << m_Type << endl;
            break;
    }
    switch (DebugLevel) {
        case 0:
            return;
        case 1:
            cerr << "Event: " << EventName << endl;
            return;
        case 2:
            {
                int IsMouse;
                IsMouseEvent(&IsMouse);
                if (IsMouse) {
                    cerr << "Event: " << EventName << 
                            "( Pos: (" << m_Pos.x << ", " << m_Pos.y << "), " << 
                            "Button state: " << m_ButtonState << ")" << endl;
                } else {
                    cerr << "Event: " << EventName << endl;
                }
            }
    }
}

