//
// $Id$
//

#ifndef _AVGConradRelais_H_
#define _AVGConradRelais_H_

#include "IAVGConradRelais.h"

#include <paintlib/plpoint.h>
#include <directfb.h>

class IAVGNode;


//46f24b63-aaaf-4882-9749-7a4c7809b9bf
#define AVGCONRADRELAIS_CID \
{ 0x46f24b63, 0xaaaf, 0x4882, { 0x97, 0x49, 0x7a, 0x4c, 0x78, 0x09, 0xb9, 0xbf }}

#define AVGCONRADRELAIS_CONTRACTID "@c-base.org/avgconradrelais;1"

class AVGConradRelais: public IAVGConradRelais
{
    public:
        AVGConradRelais ();
        virtual ~AVGConradRelais ();

        NS_DECL_ISUPPORTS
        NS_DECL_IAVGCONRADRELAIS
       
        void init(int port);
        void send();
	
    private:
        void sendCmd(unsigned char  a, unsigned char b, unsigned char c);
        void initBoard();
        unsigned char m_State[256];
        bool m_IsInitialized;
        int m_Port;
        int m_File;
        int m_NumCards;
};

#endif //_AVGConradRelais_H_
