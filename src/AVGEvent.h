//
// $Id$
//

#ifndef _AVGEvent_H_
#define _AVGEvent_H_

#include "IAVGEvent.h"

#include <paintlib/plpoint.h>
#include <directfb.h>

class IAVGNode;

//5237f8ca-a849-43fa-910a-5daeb972b94d
#define AVGEVENT_CID \
{ 0x5237f8ca, 0xa849, 0x43fa, { 0x91, 0x0a, 0x5d, 0xae, 0xb9, 0x72, 0xb9, 0x4d } }

#define AVGEVENT_CONTRACTID "@c-base.org/avgevent;1"


class AVGEvent: public IAVGEvent
{
    public:
        AVGEvent ();
        virtual ~AVGEvent ();
        void init(int type, const PLPoint& pos, 
            int buttonsPressed);
        bool init(const DFBWindowEvent& dfbWEvent);
        void setNode(IAVGNode* pNode);

        void dump(int DebugLevel);

		NS_DECL_ISUPPORTS

        NS_DECL_IAVGEVENT

    private:
        IAVGNode * m_pNode;
        int m_Type;
        PLPoint m_Pos;
        DFBInputDeviceButtonIdentifier m_ButtonId;
        DFBInputDeviceButtonMask m_ButtonsPressed;
        DFBInputDeviceKeySymbol m_KeySym;
        DFBInputDeviceModifierMask m_KeyMods;
};

#endif //_AVGEvent_H_
