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


#ifndef _AVGWindowEvent_h_
#define _AVGWindowEvent_h_

#include "AVGEvent.h"
#include "IAVGWindowEvent.h"

//be4b87b0-8c41-45af-9973-26ff4901705c
#define AVGWINDOWEVENT_CID \
{ 0xbe4b87b0, 0x8c41, 0x45af, { 0x99, 0x73, 0x26, 0xff, 0x49, 0x01, 0x70, 0x5c } }

#define AVGWINDOWEVENT_CONTRACTID "@c-base.org/avgwindowevent;1"

class AVGWindowEvent : public AVGEvent, public IAVGWindowEvent {
    public:
        AVGWindowEvent();
        virtual ~AVGWindowEvent();
        void init(int eventType, int width, int height);
        void init(int eventType, bool bFullscreen);

        NS_DECL_ISUPPORTS
        NS_DECL_IAVGWINDOWEVENT
        
        bool IsFullscreen(); 
        int getWidth();
        int getHeight();

    private:
        bool m_bFullscreen; 
        int m_Width;
        int m_Height;
};

#endif

