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

#ifndef _AVGMouseEvent_h_
#define _AVGMouseEvent_h_

#include "AVGEvent.h"
#include "IAVGMouseEvent.h"

//1f7de392-1e8b-4e84-99d5-05ede101428c
#define AVGMOUSEEVENT_CID \
{ 0x1f7de392, 0x1e8b, 0x4e84, { 0x99, 0xd5, 0x05, 0xed, 0xe1, 0x01, 0x42, 0x8c } }

#define AVGMOUSEEVENT_CONTRACTID "@c-base.org/avgmouseevent;1"

class AVGNode;

class AVGMouseEvent : public AVGEvent, public IAVGMouseEvent {
    public:
        AVGMouseEvent();
        virtual ~AVGMouseEvent();
        void init(int eventType,
                bool leftButtonState, bool middleButtonState, bool rightButtonState,
                int xPosition, int yPosition, int button);
        
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGMOUSEEVENT
       
        bool getLeftButtonState();
        bool getMiddleButtonState();
        bool getRightButtonState();
        int getXPosition();
        int getYPosition();
        int getButton();

        void setElement(AVGNode * pNode);
        virtual void trace();
        
    private:
        bool m_LeftButtonState;
        bool m_MiddleButtonState;
        bool m_RightButtonState;
        int m_XPosition;
        int m_YPosition;
        int m_Button; // only used in button events
        IAVGNode * m_pNode;
};

#endif

