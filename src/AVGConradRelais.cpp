//
// $Id$
//

#include "AVGConradRelais.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>
#include <string>

#include "nsMemory.h"

#ifdef XPCOM_GLUE
#include "nsXPCOMGlue.h"
#endif

using namespace std;

AVGConradRelais::AVGConradRelais()
    : m_IsInitialized(false),
      m_File(-1),
      m_NumCards(0)
{
    NS_INIT_ISUPPORTS();
}

AVGConradRelais::~AVGConradRelais()
{
    if (m_File != -1) {
        close(m_File);
    }
}

NS_IMPL_ISUPPORTS1_CI(AVGConradRelais, IAVGConradRelais);

NS_IMETHODIMP AVGConradRelais::GetNumCards(PRInt16 *_retval)
{
    *_retval = m_NumCards;
    return NS_OK;
}

NS_IMETHODIMP AVGConradRelais::Set(PRInt16 card, PRInt16 index, PRBool bOn)
{
    if (m_File != -1) {
        unsigned char bitMask = (unsigned char)(pow((double)2, index));
        if (bOn) {
            m_State[card] |= bitMask;
        } else {
            m_State[card] &= 255-bitMask;
        }
        return NS_OK;
    }
}

NS_IMETHODIMP AVGConradRelais::Get(PRInt16 card, PRInt16 index, PRBool *_retval)
{
    unsigned char bitMask = (unsigned char)(pow((double)2, index));
    *_retval = ((bitMask & m_State[card]) == bitMask);
    return NS_OK;
}

void AVGConradRelais::init(int port)
{
    m_Port = port;
    stringstream s;
    s << "/dev/ttyS" << m_Port;
    m_File = open(s.str().c_str(), O_RDWR | O_NOCTTY | O_NDELAY); //O_NONBLOCK);
    if (m_File == -1) {
        
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, "Could not open " << s.str() 
                << " for conrad relais card (Reason:'" << strerror(errno) << "'). Disabling.");
    } else {
        initBoard();
    }
}

void AVGConradRelais::send()
{
    for (int i=0; i<m_NumCards; i++) {
        sendCmd(3,i+1,m_State[i]);
    }
}

void AVGConradRelais::initBoard() 
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
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "No Conrad Relais cards detected. Disabling relais output.");
        close(m_File);
        m_File = -1;
    }
}

void AVGConradRelais::sendCmd(unsigned char  a, unsigned char b, unsigned char c) 
{
    unsigned char Buffer[4];
    Buffer[0] = a;
    Buffer[1] = b;
    Buffer[2] = c;
    Buffer[3] = Buffer[0]^Buffer[1]^Buffer[2];

    ssize_t BytesWritten = write(m_File, Buffer, 4);
    if (BytesWritten != 4) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "Could not send data to conrad relais card. Disabling.");
        close(m_File);
        m_File = -1;
    }
}


