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

#ifndef _AVGKeyEvent_h_
#define _AVGKeyEvent_h_

#include "AVGEvent.h"
#include "IAVGKeyEvent.h"

#include <string>

//e212dcc6-abee-4f27-8b69-157c95770951
#define AVGKEYEVENT_CID \
{ 0xe212dcc6, 0xabee, 0x4f27, { 0x8b, 0x69, 0x15, 0x7c, 0x95, 0x77, 0x09, 0x51 } }

#define AVGKEYEVENT_CONTRACTID "@c-base.org/avgkeyevent;1"

class AVGKeyEvent : public AVGEvent, public IAVGKeyEvent {
    public:
        AVGKeyEvent();
        virtual ~AVGKeyEvent();
        void init(int eventType, unsigned char scanCode, int keyCode, 
                const std::string& keyString, int modifiers);

        NS_DECL_ISUPPORTS
        NS_DECL_IAVGKEYEVENT

        unsigned char getScanCode();
        int getKeyCode();
        const std::string& getKeyString();
        int getModifiers();

        void trace();
        
    private: 
        unsigned char m_ScanCode; 
        long m_KeyCode;
        std::string m_KeyString;
        int m_Modifiers;
};

#endif

