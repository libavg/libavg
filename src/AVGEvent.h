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

#ifndef _AVGEvent_H_
#define _AVGEvent_H_

#include "IAVGEvent.h"

#include <functional>

class IAVGNode;
// 96aa892d-4d36-486a-9b73-2d7dd047e9e5
#define AVGEVENT_CID \
{ 0x96aa892d, 0x4d36, 0x486a, { 0x9b, 0x73, 0x2d, 0x7d, 0xd0, 0x47, 0xe9, 0xe5 } }

#define AVGEVENT_CONTRACTID "@c-base.org/avgevent;1"

class AVGEvent: public IAVGEvent {
    public:
        AVGEvent();
        void init (int type, int when=-1);
        virtual ~AVGEvent();
        
        int getWhen() const;
        int getType() const;
       
        virtual void trace();
        
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGEVENT
        
    private:
        int m_When;
        int m_Type;
};

// Functor to compare two EventPtrs chronologically
typedef AVGEvent * AVGEventPtr;
struct isEventAfter:std::binary_function<AVGEventPtr, AVGEventPtr, bool> {
    bool operator()(const AVGEventPtr & x, const AVGEventPtr & y) const {
        return x->getWhen() > y->getWhen();
    }
};

/*
class AVGEvent: public IAVGEvent
{
    public:
        AVGEvent ();
        virtual ~AVGEvent ();
        void init(int type, const PLPoint& pos, 
            int buttonsPressed);
        bool init(const DFBWindowEvent& dfbWEvent);
        void setNode(IAVGNode* pNode);

        void dump();

		NS_DECL_ISUPPORTS
        NS_DECL_IAVGEVENT

        int getType();
        int getKeySym();
        
    private:
        int m_Type;
        PLPoint m_Pos;
        DFBInputDeviceButtonIdentifier m_ButtonId;
        DFBInputDeviceButtonMask m_ButtonsPressed;
        DFBInputDeviceKeySymbol m_KeySym;
        DFBInputDeviceModifierMask m_KeyMods;
};
*/
#endif //_AVGEvent_H_
