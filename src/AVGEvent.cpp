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
//=============================================================================//

#include "AVGEvent.h"
#include "AVGPlayer.h"
#include "AVGTimeSource.h"
#include "AVGLogger.h"

#include "nsMemory.h"

#include <iostream>
#include <sstream>

AVGEvent::AVGEvent()
    : m_When(-1),
      m_Type(-1)
{
    NS_INIT_ISUPPORTS();
}

AVGEvent::~AVGEvent()
{
}

void AVGEvent::init(int type, int when)
{
    m_Type = type;
    if (when == -1) {
        m_When = AVGTimeSource::get()->getCurrentTicks();
    } else {
        m_When = when;
    }
}

NS_IMPL_ISUPPORTS1_CI(AVGEvent, IAVGEvent);

/* attribute long when; */
NS_IMETHODIMP AVGEvent::GetWhen(PRInt32 *aWhen) 
{
    *aWhen = getWhen();
    return NS_OK;
}

/* attribute long type; */
NS_IMETHODIMP AVGEvent::GetType(PRInt32 *aType) 
{
    *aType = getType();
    return NS_OK;
}

int AVGEvent::getWhen() const
{
    return m_When;
}

int AVGEvent::getType() const
{
    return m_Type;
}

void AVGEvent::trace()
{
    switch(m_Type) {
        case KEY_UP:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "KEY_UP");
            break;
        case KEY_DOWN:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "KEY_DOWN");
            break;
        case MOUSE_MOTION:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSE_MOTION");
            break;
        case MOUSE_BUTTON_UP:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSE_BUTTON_UP");
            break;
        case MOUSE_BUTTON_DOWN:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSE_BUTTON_DOWN");
            break;
        case MOUSE_OVER:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSE_OVER");
            break;
        case MOUSE_OUT:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSE_OUT");
            break;
        case RESIZE:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "RESIZE");
            break;
        case QUIT:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "QUIT");
            break;
    }
}

