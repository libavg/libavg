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

#include "AVGKeyEvent.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <nsMemory.h>

#include <iostream>
#include <sstream>

using namespace std;

AVGKeyEvent::AVGKeyEvent()
{
}

AVGKeyEvent::~AVGKeyEvent()
{
}

    

void AVGKeyEvent::init(int eventType, unsigned char scanCode, int keyCode, 
                const string& keyString, int modifiers)
{
    AVGEvent::init(eventType);
    m_ScanCode = scanCode;
    m_KeyCode = keyCode;
    m_KeyString = keyString;
    m_Modifiers = modifiers;
}

NS_IMPL_ISUPPORTS2_CI(AVGKeyEvent, IAVGKeyEvent, IAVGEvent);

/* readonly attribute char scanCode; */
NS_IMETHODIMP AVGKeyEvent::GetScanCode(unsigned char *aScanCode)
{
    *aScanCode = m_ScanCode;
    return NS_OK;
}

/* readonly attribute long keyCode; */
NS_IMETHODIMP AVGKeyEvent::GetKeyCode(PRInt32 *aKeyCode)
{
    *aKeyCode = m_KeyCode;
    return NS_OK;
}

/* readonly attribute string keyString; */
NS_IMETHODIMP AVGKeyEvent::GetKeyString(char * *aKeyString)
{
    *aKeyString = (char*)nsMemory::Clone(m_KeyString.c_str(), m_KeyString.length()+1);
    return NS_OK;
}

/* readonly attribute long modifiers; */
NS_IMETHODIMP AVGKeyEvent::GetModifiers(PRInt32 *aModifiers)
{
    *aModifiers = m_Modifiers;
    return NS_OK;
}

unsigned char AVGKeyEvent::getScanCode()
{
    return m_ScanCode;
}

int AVGKeyEvent::getKeyCode()
{
    return m_KeyCode;
}

const std::string& AVGKeyEvent::getKeyString()
{
    return m_KeyString;
}

int AVGKeyEvent::getModifiers()
{
    return m_Modifiers;
}

void AVGKeyEvent::trace()
{
    AVGEvent::trace();
    AVG_TRACE(AVGPlayer::DEBUG_EVENTS2, "Scancode: " << m_ScanCode << ", Keycode: " << m_KeyCode 
            << ", KeyString: " << m_KeyString << ", Modifiers: " << m_Modifiers);
}
