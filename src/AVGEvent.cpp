//
// $Id$
//

#include "AVGEvent.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <paintlib/pldebug.h>

#include "nsMemory.h"

#include <iostream>
#include <sstream>
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
        int buttonsPressed)
{
    m_Type = type;
    m_Pos = pos;
    m_ButtonsPressed = (DFBInputDeviceButtonMask)buttonsPressed;
}

bool AVGEvent::init(const DFBWindowEvent& dfbWEvent)
{
    switch(dfbWEvent.type) {
        case DWET_KEYDOWN:
            m_Type = KEYDOWN;
            m_KeySym = dfbWEvent.key_symbol;
            m_KeyMods = dfbWEvent.modifiers;
            break;
        case DWET_KEYUP:
            m_Type = KEYUP;
            m_KeySym = dfbWEvent.key_symbol;
            m_KeyMods = dfbWEvent.modifiers;
            break;
        case DWET_BUTTONDOWN:
            m_Type = MOUSEDOWN;
            m_Pos = PLPoint(dfbWEvent.cx, dfbWEvent.cy);
            m_ButtonId = dfbWEvent.button;
            m_ButtonsPressed = dfbWEvent.buttons;
            m_KeyMods = dfbWEvent.modifiers;
            break;
        case DWET_BUTTONUP:
            m_Type = MOUSEUP;
            m_Pos = PLPoint(dfbWEvent.cx, dfbWEvent.cy);
            m_ButtonId = dfbWEvent.button;
            m_ButtonsPressed = dfbWEvent.buttons;
            m_KeyMods = dfbWEvent.modifiers;
            break;
        case DWET_MOTION:
            m_Type = MOUSEMOVE;
            m_Pos = PLPoint(dfbWEvent.cx, dfbWEvent.cy);
            m_ButtonsPressed = dfbWEvent.buttons;
            m_KeyMods = dfbWEvent.modifiers;
            break;
/*        case DWET_WHEEL:
            m_Type = ;
            break;
*/
        default:
            return false;
    }
    return true;
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
    *_retval = (PRInt32)m_ButtonsPressed;
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
    *_retval = m_KeyMods;    
    return NS_OK;
}

int AVGEvent::getType() {
    return m_Type;
}

int AVGEvent::getKeySym() {
    return m_KeySym;
}

void AVGEvent::dump()
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
    
    AVG_TRACE(AVGPlayer::DEBUG_EVENTS, EventName);
    int IsMouse;
    IsMouseEvent(&IsMouse);
    if (IsMouse) {
        AVG_TRACE(AVGPlayer::DEBUG_EVENTS2, EventName << 
                "( Pos: (" << m_Pos.x << ", " << m_Pos.y << "), " << 
                "Buttons pressed: " << m_ButtonsPressed << ")");
    } else {
        if (m_Type == AVGEvent::KEYDOWN) {
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS2, EventName << 
                    ", Key symbol: " << m_KeySym << ", Key modifiers: " << 
                    m_KeyMods);
        } else {
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS2, EventName);
        }
    }
}

