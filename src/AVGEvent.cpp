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

int AVGEvent::s_CurCounter = 0;

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
    // Make sure two events with the same timestamp are ordered correctly.
    s_CurCounter++;
    m_Counter = s_CurCounter;
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
        case KEYUP:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "KEYUP");
            break;
        case KEYDOWN:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "KEYDOWN");
            break;
        case MOUSEMOTION:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSEMOTION");
            break;
        case MOUSEBUTTONUP:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSEBUTTONUP");
            break;
        case MOUSEBUTTONDOWN:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSEBUTTONDOWN");
            break;
        case MOUSEOVER:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSEOVER");
            break;
        case MOUSEOUT:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "MOUSEOUT");
            break;
        case RESIZE:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "RESIZE");
            break;
        case QUIT:
            AVG_TRACE(AVGPlayer::DEBUG_EVENTS, "QUIT");
            break;
    }
}

