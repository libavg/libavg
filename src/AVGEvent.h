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
        
        friend struct isEventAfter;

    private:
        int m_When;
        int m_Type;
        int m_Counter;

        static int s_CurCounter;
};

// Functor to compare two EventPtrs chronologically
typedef AVGEvent * AVGEventPtr;
struct isEventAfter:std::binary_function<AVGEventPtr, AVGEventPtr, bool> {
    bool operator()(const AVGEventPtr & x, const AVGEventPtr & y) const {
        if (x->getWhen() == y->getWhen()) {
            return x->m_Counter > y->m_Counter;
        }
        return x->getWhen() > y->getWhen();
    }
};

#endif //_AVGEvent_H_
