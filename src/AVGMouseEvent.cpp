//=============================================================================
//
// Original code Copyright (C) 2003, ART+COM AG Berlin
//
// Released under LGPL.
//
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
//=============================================================================

#include "AVGMouseEvent.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGNode.h"

#include <nsMemory.h>

#include <iostream>
#include <sstream>

using namespace std;

AVGMouseEvent::AVGMouseEvent()
    : m_pNode(0)
{
}

AVGMouseEvent::~AVGMouseEvent()
{
}

void AVGMouseEvent::init(int eventType,
        bool leftButtonState, bool middleButtonState, bool rightButtonState,
        int xPosition, int yPosition, int button)
{
    AVGEvent::init(eventType);
    m_LeftButtonState = leftButtonState;
    m_MiddleButtonState = middleButtonState;
    m_RightButtonState = rightButtonState;
    m_XPosition = xPosition;
    m_YPosition = yPosition;
    if (eventType == MOUSE_MOTION) {
        m_Button = 0;
    } else {
        m_Button = button;
    }
}

NS_IMPL_ISUPPORTS2_CI(AVGMouseEvent, IAVGMouseEvent, IAVGEvent);

NS_IMETHODIMP AVGMouseEvent::GetElement(IAVGNode **_retval)
{
    NS_IF_ADDREF(m_pNode);
    *_retval = m_pNode;
    return NS_OK;
}

/* readonly attribute boolean leftButtonState; */
NS_IMETHODIMP AVGMouseEvent::GetLeftButtonState(PRBool *aLeftButtonState)
{
    *aLeftButtonState = m_LeftButtonState;
    return NS_OK;
}

/* readonly attribute boolean middleButtonState; */
NS_IMETHODIMP AVGMouseEvent::GetMiddleButtonState(PRBool *aMiddleButtonState)
{
    *aMiddleButtonState = m_MiddleButtonState;
    return NS_OK;
}

/* readonly attribute boolean rightButtonState; */
NS_IMETHODIMP AVGMouseEvent::GetRightButtonState(PRBool *aRightButtonState)
{
    *aRightButtonState = m_RightButtonState;
    return NS_OK;
}

/* readonly attribute long xPosition; */
NS_IMETHODIMP AVGMouseEvent::GetXPosition(PRInt32 *aXPosition)
{
    *aXPosition = m_XPosition;
    return NS_OK;
}

/* readonly attribute long yPosition; */
NS_IMETHODIMP AVGMouseEvent::GetYPosition(PRInt32 *aYPosition)
{
    *aYPosition = m_YPosition;
    return NS_OK;
}

/* readonly attribute long button; */
NS_IMETHODIMP AVGMouseEvent::GetButton(PRInt32 *aButton)
{
    *aButton = m_Button;
    return NS_OK;
}

bool AVGMouseEvent::getLeftButtonState()
{
    return m_LeftButtonState;
}

bool AVGMouseEvent::getMiddleButtonState()
{
    return m_MiddleButtonState;
}

bool AVGMouseEvent::getRightButtonState()
{
    return m_RightButtonState;
}

int AVGMouseEvent::getXPosition()
{
    return m_XPosition;
}

int AVGMouseEvent::getYPosition()
{
    return m_YPosition;
}

int AVGMouseEvent::getButton()
{
    return m_Button;
}

void AVGMouseEvent::setElement(AVGNode * pNode)
{
    m_pNode = pNode;
}

void AVGMouseEvent::trace()
{
    AVGEvent::trace();
    AVG_TRACE(AVGPlayer::DEBUG_EVENTS2, "pos: (" << m_XPosition << ", " << m_YPosition << ")" 
            << ", button: " << m_Button);
}
