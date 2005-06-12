//
// $Id$
//

#ifndef _ConradRelais_H_
#define _ConradRelais_H_

#include "../base/IFrameListener.h"

#include "../player/Player.h"

namespace avg {

/**
 * Interface to one or more conrad relais card connected to a serial 
 * port. Per card, up to eight 220V devices can be turned on or off.
 */
class ConradRelais: public IFrameListener
{
    public:
        ConradRelais ();
        virtual ~ConradRelais ();

        /**
         * Returns the number of cards connected to the serial port.
         */
        int getNumCards();
        /**
         * Sets or resets one of the relais. index selects the relais
         * to set.
         */
        void set(int card, int index, bool bOn);
        /**
         * Returns the state of one of the relais. index selects the 
         * relais to query.
         */
        bool get(int card, int index);

        void init(Player * pPlayer, int port);
        void send();

        virtual void onFrameEnd();

    private:
        void sendCmd(unsigned char a, unsigned char b, unsigned char c);
        void initBoard();
        unsigned char m_State[256];
        bool m_IsInitialized;
        int m_Port;
        int m_File;
        int m_NumCards;
};

}
#endif 
