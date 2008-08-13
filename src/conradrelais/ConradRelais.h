//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
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
        ConradRelais (Player * pPlayer, int port);
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

        Player * m_pPlayer;
};

}
#endif 
