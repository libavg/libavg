//
// $Id$
//

%module avg
%{
#include "../conradrelais/ConradRelais.h"
#include "../base/IFrameListener.h"
#include "../player/Player.h"
%}

namespace avg {

/**
 * Interface to one or more conrad relais cards connected to a serial 
 * port. Per card, up to eight 220V devices can be turned on or off.
 */
class ConradRelais
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
};

}
