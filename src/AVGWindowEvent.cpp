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

#include "AVGWindowEvent.h"

#include <nsMemory.h>

AVGWindowEvent::AVGWindowEvent()
{
}

AVGWindowEvent::~AVGWindowEvent()
{
}

void AVGWindowEvent::init(int eventType, int width, int height)
{
    AVGEvent::init(eventType);
    m_Width = width;
    m_Height = height;
}

void AVGWindowEvent::init(int eventType, bool bFullscreen)
{
    AVGEvent::init(eventType);
    m_bFullscreen = bFullscreen;
}

NS_IMPL_ISUPPORTS2_CI(AVGWindowEvent, IAVGWindowEvent, IAVGEvent);

/* readonly attribute boolean fullscreen; */
NS_IMETHODIMP AVGWindowEvent::GetFullscreen(PRBool *aFullscreen)
{
    *aFullscreen = m_bFullscreen;
    return NS_OK;
}

/* readonly attribute long width; */
NS_IMETHODIMP AVGWindowEvent::GetWidth(PRInt32 *aWidth)
{
    *aWidth = m_Width;
    return NS_OK;
}

/* readonly attribute long height; */
NS_IMETHODIMP AVGWindowEvent::GetHeight(PRInt32 *aHeight)
{
    *aHeight = m_Height;
    return NS_OK;
}

bool AVGWindowEvent::IsFullscreen()
{
    return m_bFullscreen;
}

int AVGWindowEvent::getWidth()
{
    return m_Width;
}

int AVGWindowEvent::getHeight()
{
    return m_Height;
}


