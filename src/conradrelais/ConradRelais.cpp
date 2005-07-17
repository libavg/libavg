//
// $Id$
//

#include "ConradRelais.h"
#include "../base/Logger.h"

#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <math.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace avg {

ConradRelais::ConradRelais(Player * pPlayer, int port)
    : m_IsInitialized(false),
      m_File(-1),
      m_NumCards(0)
{
    m_Port = port;
    stringstream s;
    s << "/dev/ttyS" << m_Port;
    m_File = open(s.str().c_str(), O_RDWR | O_NOCTTY | O_NDELAY); //O_NONBLOCK);
    if (m_File == -1) {
        AVG_TRACE(Logger::ERROR, "Could not open " << s.str() 
                << " for conrad relais card (Reason:'" << strerror(errno) 
                << "'). Disabling.");
    } else {
        initBoard();
        if (m_File != -1) {
            pPlayer->registerFrameListener(this);
        }
    }
}

ConradRelais::~ConradRelais()
{
    if (m_File != -1) {
        close(m_File);
    }
}

int ConradRelais::getNumCards()
{
    return m_NumCards;
}

void ConradRelais::set(int card, int index, bool bOn)
{
    if (m_File != -1) {
        unsigned char bitMask = (unsigned char)(pow((double)2, index));
        if (bOn) {
            m_State[card] |= bitMask;
        } else {
            m_State[card] &= 255-bitMask;
        }
    }
}

bool ConradRelais::get(int card, int index)
{
    unsigned char bitMask = (unsigned char)(pow((double)2, index));
    return (bitMask & m_State[card]) == bitMask;
}

void ConradRelais::send()
{
    for (int i=0; i<m_NumCards; i++) {
        sendCmd(3,i+1,m_State[i]);
    }
}

void ConradRelais::initBoard() 
{
    fcntl(m_File, F_SETFL, 0);

    /* get the current options */
    struct termios options;
    tcgetattr(m_File, &options);

    /* set in and out speed */
    cfsetispeed(&options, B19200);
    cfsetospeed(&options, B19200);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE; /* Mask the character size bits */
    options.c_cflag |= CS8;    /* Select 8 data bits */

    /* set raw input, 1 second timeout */
    options.c_cflag     |= (CLOCAL | CREAD);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    /* set the options */
    tcsetattr(m_File, TCSAFLUSH, &options);

    /*
     * init the circuit board
     */
    sendCmd(1, 1, 0);
    fcntl(m_File, F_SETFL, FNDELAY);
    fcntl(m_File, F_SETFL, 0);
    unsigned char rbuf[4];
    m_NumCards = 0;
    bool bOk = true;
    // One read per card should succeed.
    while (bOk) {
        ssize_t rc = read(m_File, rbuf, 4);
        if (rc != 4  || rbuf[0] != 254 || rbuf[1] != m_NumCards+1) {
//            cerr << "rc: " << rc << ", rbuf: " << (int)rbuf[0] << ":" << (int)rbuf[1] 
//		 << ":" << (int)rbuf[2] << ":" << (int)rbuf[3] << endl;
	    bOk = false;
        } else {
//	    cerr << "card init." << endl;
            m_NumCards++;
        }
    }
    if (m_NumCards == 0) {
        AVG_TRACE(Logger::ERROR, 
                "No Conrad Relais cards detected. Disabling relais output.");
        close(m_File);
        m_File = -1;
    }
}

void ConradRelais::sendCmd(unsigned char  a, unsigned char b, unsigned char c) 
{
    unsigned char Buffer[4];
    Buffer[0] = a;
    Buffer[1] = b;
    Buffer[2] = c;
    Buffer[3] = Buffer[0]^Buffer[1]^Buffer[2];

    ssize_t BytesWritten = write(m_File, Buffer, 4);
    if (BytesWritten != 4) {
        AVG_TRACE(Logger::ERROR, 
                "Could not send data to conrad relais card. Disabling.");
        close(m_File);
        m_File = -1;
    }
}

void ConradRelais::onFrameEnd()
{
    send();
}

}
